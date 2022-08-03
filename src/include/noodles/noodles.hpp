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

namespace noodles
{

namespace impl {

class Noodles
{
public:

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

} // namespace noodles
