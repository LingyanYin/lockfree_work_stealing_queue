#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>

#include "threadpool.h"

// for debug
#include <unistd.h>

thread_local LockFreeWorkStealingQueue* ThreadPool::local_queue;
thread_local unsigned int ThreadPool::idx;

void sum_over(std::vector<int>& v, int& res) {
    res = std::accumulate(v.begin(), v.end(), 0);
}

int main () {
    constexpr unsigned int tasknum = 8;
    std::vector<int> v { 1,2,3,4,5,6,7,8,9 };
    std::vector<int> res(tasknum, -1);
    std::vector<std::future<void>> futures;

    {
        ThreadPool threadpool;
        // std::cout << "before threadpool submit\n";
        for (auto i = 0; i < tasknum; ++i) {
            futures.push_back(threadpool.submit(sum_over, std::ref(v), std::ref(res.at(i))));
        }

        for (auto i = 0; i < tasknum; ++i) {
            futures[i].wait();
        }
    }

    std::cout << "threadpool was destroyed.\n";

    for (auto i = 0; i < tasknum; ++i) {
        futures[i].get();
        if (res[i] != res[0]) {
            std::cout << "res[" << i << "]=" << res[i] << std::endl;
            std::cout << "res[0]=" << res[0] << std::endl;
        }
        assert(res[i] == res[0]);
    }

    std::cout << "All results are " << res[0] << std::endl;

    return 0;
}
