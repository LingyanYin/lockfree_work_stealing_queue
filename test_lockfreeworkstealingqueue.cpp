#include "lockfreeworkstealingqueue.h"
#include "threadwrapper.h"
#include <iostream>
#include <algorithm>
#include <numeric>

std::vector<int> v { 1,2,3,4,5,6,7,8,9 };

int sumOver() {
    return std::accumulate(v.begin(), v.end(), 0);
}

void callWSQ(LockFreeWorkStealingQueue& q) {
    FunctionWrapper f;
    do {
        if (q.try_steal_front(f)) {
            std::cout << "threadid: " << std::this_thread::get_id() << " sum: " << f() << std::endl;
        }
        std::this_thread::yield();
    } while(true);
}

int main () {
    std::vector<std::thread> threads;
    std::vector<LockFreeWorkStealingQueue> wsq(4);
    ThreadWrapper joiner(threads);

    for (auto i = 0; i < 4; ++i) {
        for (auto j = 0; j < 10; ++j)
            wsq[i].push_back(FunctionWrapper(sumOver));
        FunctionWrapper f;
        if(wsq[i].try_pop_back(f))
            std::cout << "main thread" << f() << std::endl;
        threads.push_back(std::thread(callWSQ, std::ref(wsq[i])));
    }

    return 0;
}
