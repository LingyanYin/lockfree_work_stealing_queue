#include "lockfreeworkstealingqueue.h"
#include "threadwrapper.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cassert>

std::atomic<unsigned int> gCounter{0};
constexpr unsigned int NUM = 256;

void sumOver(const std::vector<int>& v, int& res) {
    res = std::accumulate(v.begin(), v.end(), 0);
}

void callWSQ(LockFreeWorkStealingQueue& q) {
    FunctionWrapper f;
    unsigned int tmp = 0;
    do {
        if (q.try_steal_front(f)) {
            f();
            gCounter.fetch_add(1, std::memory_order_release);
            // std::cout << "threadid: " << std::this_thread::get_id() << " gCounter: " << gCounter.load(std::memory_order_acquire) << std::endl;
        }
        std::this_thread::yield();
        tmp = gCounter.load(std::memory_order_acquire);
    } while(tmp < NUM);
}

int main () {
    std::vector<int> v { 1,2,3,4,5,6,7,8,9 };
    std::vector<int> res(NUM, -1);
    std::vector<std::thread> threads;
    std::vector<LockFreeWorkStealingQueue> wsq(4);

    {
        ThreadWrapper joiner(threads);

        for (auto i = 0; i < 4; ++i) {
            for (auto j = 0; j < NUM / 4; ++j) {
                // std::cout << "push_back addr: " << &res[i*(NUM/4)+j] << std::endl;
                wsq[i].push_back(FunctionWrapper(sumOver, std::ref(v), std::ref(res.at(i*(NUM/4)+j)))); 
            }
            threads.push_back(std::thread(callWSQ, std::ref(wsq[i])));
        }

        for (auto i = 0; i < 8; ++i) {
            threads.push_back(std::thread(callWSQ, std::ref(wsq[i % 4])));
        }

        FunctionWrapper f;
        for (auto i = 0; i < 4; ++i) {
            for (auto j = 0; j < NUM / 4; ++j) {
                if(wsq[i].try_pop_back(f)) {
                    f();
                    gCounter.fetch_add(1, std::memory_order_release);
                }
            }
        }
    } 

    for (auto i = 0; i < 4; ++i) {
        for (auto j = 0; j < NUM / 4; ++j) {
            if (res[j*i+j] != res[0]) {
                std::cout << "res[" << j*i+j << "]=" << res[j*i+j] << std::endl;
                std::cout << "res[0]=" << res[0] << std::endl;
            }
            assert(res[j*i+j] == res[0]);
            //std::cout << "res[" << i*(NUM/4)+j << "]=" << res[i*(NUM/4)+j] << std::endl;
        }
    }

    std::cout << "All results are " << res[0] << std::endl;
    std::cout << "main thread: gCounter=" << gCounter.load(std::memory_order_acquire) << std::endl;
    assert(gCounter.load(std::memory_order_acquire) == NUM);

    return 0;
}
