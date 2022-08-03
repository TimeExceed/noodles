#pragma once

#include <mutex>
#include <condition_variable>
#include <cinttypes>

namespace noodles {

class Semaphore
{
public:
    explicit Semaphore(size_t init) noexcept;
    ~Semaphore();

    Semaphore(const Semaphore&) =delete;
    Semaphore& operator=(const Semaphore&) =delete;
    Semaphore(Semaphore&&) =default;
    Semaphore& operator=(Semaphore&&) =default;

    void post() noexcept;
    void wait() noexcept;

    template<class Duration>
    std::cv_status wait_for(Duration duration) noexcept {
        std::unique_lock<std::mutex> lock(mMutex);
        while (mAvailable == 0) {
            auto res = mCV.wait_for(lock, duration);
            if (res == std::cv_status::timeout) {
                return std::cv_status::timeout;
            }
        }
        --mAvailable;
        return std::cv_status::no_timeout;
    }

private:
    std::mutex mMutex;
    std::condition_variable mCV;
    size_t mAvailable;
};

} // namespace noodles