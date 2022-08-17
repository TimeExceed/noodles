#pragma once

#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <deque>
#include <optional>
#include <cinttypes>

namespace noodles {
namespace naive {

class ThreadPool
{
public:
    typedef std::function<void()> Task;

    explicit ThreadPool(std::size_t concurrency, std::size_t slot_size) noexcept;
    ~ThreadPool();

    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void submit(Task) noexcept;

private:
    void worker() noexcept;

private:
    std::size_t mSlotLimit;
    std::condition_variable mCV;
    std::mutex mMutex;
    std::deque<std::optional<Task>> mSlots;
    std::deque<std::thread> mWorkers;
};

} // namespace naive
} // namespace noodles
