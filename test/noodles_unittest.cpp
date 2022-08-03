#include "noodles/noodles.hpp"
#include "testa.hpp"
#include "prettyprint.hpp"
#include <deque>
#include <iostream>

using namespace std;

void single(const string&)
{
    noodles::impl::Noodles tp(1, 1);
    promise<int> prom;
    future<int> fut = prom.get_future();
    std::function<void()> t([&]() {
        cout << "haha" << endl;
        prom.set_value(42);
    });
    tp.submit(t);
    fut.wait();
    auto val = fut.get();
    TESTA_ASSERT(val == 42)
        (val)
        .issue();
}
TESTA_DEF_JUNIT_LIKE1(single);

void multiple(const string&)
{
    const size_t N = 100;
    noodles::impl::Noodles tp(10, N / 4);
    deque<promise<size_t>> proms;
    deque<future<size_t>> futs;
    for(size_t i = 0; i < N; ++i) {
        promise<size_t> prom;
        futs.push_back(std::move(prom.get_future()));
        proms.push_back(std::move(prom));
    }
    for(size_t i = 0; i < N; ++i) {
        auto& prom = proms[i];
        std::function<void()> t([=, &prom]() {
            prom.set_value(i);
        });
        tp.submit(t);
    }
    deque<size_t> results;
    for(; !futs.empty(); futs.pop_front()) {
        auto& fut = futs.front();
        fut.wait();
        results.push_back(fut.get());
    }
    for(size_t i = 0; i < N; ++i) {
        TESTA_ASSERT(results[i] == i)
            (i)
            (results)
            .issue();
    }
}
TESTA_DEF_JUNIT_LIKE1(multiple);
