#include "noodles/noodles.hpp"
#include "prettyprint.hpp"
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
#include <iomanip>

using namespace std;

atomic<size_t> txn_submitted;
atomic<bool> stop;

typedef function<void()> Task;

void run(
    function<void(Task)> submitter,
    size_t n_writers,
    size_t concurrency,
    size_t slot_size
) {
    stop.store(false, memory_order_relaxed);
    deque<thread> writers;
    for(size_t n = n_writers; n > 0; --n) {
        thread t([&]() {
            for(;;) {
                for(size_t n = 100; n > 0; --n) {
                    Task task([&]() {
                        chrono::duration<size_t, micro> sleep_time(1000);
                        this_thread::sleep_for(sleep_time);
                        txn_submitted.fetch_add(1, memory_order_relaxed);
                    });
                    submitter(task);
                }
                if (stop.load(memory_order_relaxed)) {
                    break;
                }
            }
        });
        writers.push_back(std::move(t));
    }
    chrono::duration<size_t, micro> probe_interval(1000000);
    this_thread::sleep_for(probe_interval);
    for(size_t n = 4; n > 0; --n) {
        auto startpoint = chrono::steady_clock::now();
        size_t from = txn_submitted.load(memory_order_relaxed);
        this_thread::sleep_for(probe_interval);
        auto endpoint = chrono::steady_clock::now();
        size_t to = txn_submitted.load(memory_order_relaxed);
        double duration = (endpoint - startpoint).count() / 1000000000.0;
        size_t cnt = to - from;
        cout
            << " #slot " << slot_size
            << " concurrency " << concurrency
            << " #writers " << n_writers
            << " count down " << n
            << " : " << cnt << " txns submitted in "
            << duration << " secs. "
            << (cnt / duration) << " txns per sec."
            << endl;
    }
    stop.store(true, memory_order_relaxed);
    for(; !writers.empty(); writers.pop_back()) {
        auto& t = writers.back();
        t.join();
    }
}

int main() {
    txn_submitted.store(0, memory_order_relaxed);
    for(size_t slot_size = 100; slot_size <= 1000; slot_size *= 2) {
        for(size_t concurrency = 2; concurrency <= 64; concurrency *= 2) {
            for(size_t n_writers = 2; n_writers <= 4; n_writers *= 2) {
                noodles::impl::Noodles tp(concurrency, slot_size);
                function<void(Task)> submitter = bind(&noodles::impl::Noodles::submit, &tp, placeholders::_1);
                run(std::move(submitter), n_writers, concurrency, slot_size);
            }
        }
    }
    return 0;
}
