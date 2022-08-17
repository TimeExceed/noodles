#include "noodles/naive.hpp"
#include <chrono>

using namespace std;

namespace noodles {
namespace naive {

ThreadPool::ThreadPool(size_t concurrency, size_t slot_size) noexcept
:   mSlotLimit(slot_size)
{
    for(size_t i = concurrency; i > 0; --i) {
        thread t(bind(&ThreadPool::worker, this));
        mWorkers.push_back(std::move(t));
    }
}

ThreadPool::~ThreadPool()
{
    {
        lock_guard<mutex> g(mMutex);
        for(size_t i = mWorkers.size(); i > 0; --i) {
            mSlots.push_back(optional(Task()));
        }
    }
    for(; !mWorkers.empty(); mWorkers.pop_front()) {
        mWorkers.front().join();
    }
}

void ThreadPool::worker() noexcept
{
    for(;;) {
        optional<Task> opt_task;
        {
            lock_guard<mutex> g(mMutex);
            if (!mSlots.empty()) {
                std::swap(opt_task, mSlots.front());
                mSlots.pop_front();
            }
        }
        if (opt_task) {
            if (*opt_task) {
                (*opt_task)();
            } else {
                // indicator to destroying
                break;
            }
        } else {
            unique_lock<mutex> g(mMutex);
            mCV.wait(g);
        }
    }
}

void ThreadPool::submit(Task task) noexcept
{
    for(;;) {
        lock_guard<mutex> g(mMutex);
        if (mSlots.size() < mSlotLimit) {
            mSlots.emplace_back(std::move(task));
            mCV.notify_one();
            return;
        }
    }
}

} // namespace naive
} // namespace noodles
