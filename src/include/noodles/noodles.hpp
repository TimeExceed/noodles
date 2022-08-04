#pragma once

#include "semaphore.hpp"

#include <functional>
#include <tuple>
#include <optional>
#include <future>
#include <type_traits>
#include <mutex>
#include <thread>
#include <atomic>
#include <deque>
#include <vector>
#include <memory>

namespace noodles
{

namespace impl {

class Noodles
{
public:
    explicit Noodles(size_t concurrency, size_t slot_size_hint) noexcept;
    ~Noodles();

    Noodles(Noodles&&) =delete;
    Noodles& operator=(Noodles&&) =delete;
    Noodles(const Noodles&) =delete;
    Noodles& operator=(const Noodles&) =delete;

    /**
     * @brief Submit a task to the thread pool.
     *  User must assure that the thread pool is not destructing.
     *
     * @param task An async task, which will never throw exceptions.
     *  Once a task is successfully submitted, it will finally be executed.
     */
    void submit(std::function<void()> task) noexcept;

private:
    void worker() noexcept;
    bool run_on_slot(size_t i) noexcept;
    void wake_up_all() noexcept;

    struct Slot
    {
        std::mutex mMutex;
        std::function<void()> mWork;
    };
    std::vector<Slot> mSlots;
    std::deque<std::thread> mWorkers;
    std::atomic<bool> mQuit;
    Semaphore mPillow;
};

} // namespace impl

class Noodles
{
public:
    explicit Noodles(size_t concurrency, size_t slot_size_hint) noexcept;
    ~Noodles();

    Noodles(Noodles&&) =default;
    Noodles& operator=(Noodles&&) =default;
    Noodles(const Noodles&) =delete;
    Noodles& operator=(const Noodles&) =delete;

    /**
     * @brief Submit a task to the thread pool.
     *  User must assure that the thread pool is not destructing.
     *
     * @param task An async task, which will never throw exceptions.
     *  Once a task is successfully submitted, it will finally be executed.
     */
    template<class F>
    std::future<std::invoke_result_t<F>> submit(F task) noexcept
    {
        typedef std::invoke_result_t<F> R;
        std::shared_ptr<std::promise<R>> prom(new std::promise<R>());
        std::future<R> fut = prom->get_future();
        std::function<void()> t = [=]() noexcept {
            R&& res = task();
            prom->set_value(res);
        };
        mImpl->submit(std::move(t));
        return fut;
    }

private:
    std::unique_ptr<impl::Noodles> mImpl;
};

} // namespace noodles
