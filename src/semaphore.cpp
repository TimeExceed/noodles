#include "noodles/semaphore.hpp"

using namespace std;

namespace noodles {

Semaphore::Semaphore(size_t init) noexcept
:   mMutex(),
    mCV(),
    mAvailable(init)
{}

Semaphore::~Semaphore()
{}

void Semaphore::post() noexcept
{
    {
        unique_lock<mutex> lock(mMutex);
        ++mAvailable;
    }
    mCV.notify_one();
}

void Semaphore::wait() noexcept
{
    unique_lock<mutex> lock(mMutex);
    while (mAvailable == 0) {
        mCV.wait(lock);
    }
    --mAvailable;
}

} // namespace noodles
