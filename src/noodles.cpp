#include "noodles/noodles.hpp"
#include "fassert.hpp"
#include <random>
#include <chrono>
#include <algorithm>

using namespace std;

namespace noodles {

namespace impl {

const size_t WORKTIME = 30;

const size_t PRIMES[] = {
    157,
    307,
    617,
    1237,
    2473,
    4943,
    9887,
    19777,
    39551,
};

size_t calc_slot_size(size_t slot_size_hint) noexcept
{
    const size_t n = sizeof(PRIMES) / sizeof(PRIMES[0]);
    auto it = std::lower_bound(PRIMES, PRIMES + n, slot_size_hint);
    if (it < PRIMES + n) {
        return *it;
    } else {
        return PRIMES[n - 1];
    }
}

Noodles::Noodles(size_t concurrency, size_t slot_size_hint) noexcept
:   mSlots(calc_slot_size(slot_size_hint)),
    mQuit(false),
    mPillow(0)
{
    FASSERT(concurrency > 0);
    for (size_t i = 0; i < concurrency; ++i) {
        mWorkers.push_back(thread(bind(&Noodles::worker, this)));
    }
}

Noodles::~Noodles()
{
    mQuit.store(true, memory_order_relaxed);
    wake_up_all();
    for(; !mWorkers.empty(); mWorkers.pop_back()) {
        mWorkers.back().join();
    }
}

void Noodles::wake_up_all() noexcept
{
    for (size_t i = mWorkers.size(); i > 0; --i) {
        mPillow.post();
    }
}

void Noodles::worker() noexcept
{
    const size_t n_slots = mSlots.size();
    mt19937_64 rng;
    uniform_int_distribution<size_t> slot_rng(0, n_slots);
    size_t cur_slot = slot_rng(rng);
    size_t slot_step = slot_rng(rng) + 1;
    uniform_int_distribution<size_t> sleep_time_rng(1000, 10000);
    for(;;) {
        if (mQuit.load(memory_order_relaxed)) {
            break;
        }
        for(size_t work = WORKTIME; work > 0;) {
            auto done = run_on_slot(cur_slot);
            cur_slot = (cur_slot + slot_step) % n_slots;
            if (done) {
                work = WORKTIME;
            } else {
                --work;
            }
        }
        chrono::duration<size_t, micro> sleep_time(sleep_time_rng(rng));
        mPillow.wait_for(sleep_time);
        slot_step = slot_rng(rng);
    }
    for(size_t i = 0; i < n_slots; ++i) {
        run_on_slot(cur_slot);
        cur_slot = (cur_slot + slot_step) % n_slots;
    }
}

bool Noodles::run_on_slot(size_t i) noexcept
{
    function<void()> todo;
    auto& slot = mSlots[i];
    if (slot.mMutex.try_lock()) {
        lock_guard<mutex> lock(slot.mMutex, adopt_lock);
        if (!slot.mWork) {
            return false;
        } else {
            std::swap(todo, slot.mWork);
        }
    } else {
        return false;
    }
    todo();
    return true;
}

void Noodles::submit(std::function<void()> task) noexcept
{
    FASSERT(!mQuit.load(memory_order_relaxed));
    const size_t n_slots = mSlots.size();
    mt19937_64 rng;
    uniform_int_distribution<size_t> slot_rng(0, n_slots);
    size_t cur_slot = slot_rng(rng);
    size_t slot_step = slot_rng(rng);
    size_t sleep_time_bound = 1000;
    for(;;) {
        FASSERT(!mQuit.load(memory_order_relaxed));
        for(size_t work = WORKTIME; work > 0; --work) {
            auto& slot = mSlots[cur_slot];
            if (slot.mMutex.try_lock()) {
                lock_guard<mutex> lock(slot.mMutex, adopt_lock);
                if (!slot.mWork) {
                    slot.mWork = std::move(task);
                    wake_up_all();
                    return;
                }
            }
            cur_slot = (cur_slot + slot_step) % n_slots;
        }
        uniform_int_distribution<size_t> sleep_time_rng(sleep_time_bound, 2 * sleep_time_bound);
        chrono::duration<size_t, micro> sleep_time(sleep_time_rng(rng));
        this_thread::sleep_for(sleep_time);
        slot_step = slot_rng(rng);
        if (sleep_time_bound < 30000) {
            sleep_time_bound *= 2;
        }
    }
}

} // namespace impl

Noodles::Noodles(size_t concurrency, size_t slot_size_hint) noexcept
:   mImpl(new impl::Noodles(concurrency, slot_size_hint))
{}

Noodles::~Noodles()
{}

} // namespace noodles

