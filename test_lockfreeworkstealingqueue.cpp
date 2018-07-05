#include "lockfreeworkstealingqueue.h"
#include "threadwrapper.h"
#include <iostream>
#include <algorithm>
#include <numeric>

std::vector<int> v { 1,2,3,4,5,6,7,8,9 };
std::atomic<unsigned int> gCounter{0};
constexpr unsigned int NUM = 256;

int sumOver() {
    return std::accumulate(v.begin(), v.end(), 0);
}

void callWSQ(LockFreeWorkStealingQueue& q) {
    FunctionWrapper f;
    unsigned int tmp = 0;
    do {
        if (q.try_steal_front(f)) {
            // std::cout << "threadid: " << std::this_thread::get_id() << " sum: " << f() << std::endl;
            f();
            gCounter.fetch_add(1, std::memory_order_release);
            std::cout << "threadid: " << std::this_thread::get_id() << " gCounter: " << gCounter.load(std::memory_order_acquire) << std::endl;
        }
        std::this_thread::yield();
        tmp = gCounter.load(std::memory_order_acquire);
    } while(tmp < NUM);
}

int main () {
    std::vector<std::thread> threads;
    std::vector<LockFreeWorkStealingQueue> wsq(4);

    {
        ThreadWrapper joiner(threads);

        for (auto i = 0; i < 4; ++i) {
            for (auto j = 0; j < NUM / 4; ++j)
                wsq[i].push_back(FunctionWrapper(sumOver)); 
            threads.push_back(std::thread(callWSQ, std::ref(wsq[i])));
        }

        for (auto i = 0; i < 4; ++i) {
            FunctionWrapper f;
            for (auto j = 0; j < NUM / 4; ++j) {
                if(wsq[i].try_pop_back(f)) {
                    // std::cout << "main thread" << f() << std::endl;
                    f();
                    gCounter.fetch_add(1, std::memory_order_release);
                }
            }
        }
    } 

    std::cout << "main thread: gCounter=" << gCounter.load(std::memory_order_acquire) << std::endl;

    return 0;
}
