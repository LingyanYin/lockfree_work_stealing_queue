#include <thread>
#include <vector>

class ThreadWrapper {
private:
    std::vector<std::thread>& threads;
public:
    explicit ThreadWrapper(std::vector<std::thread>& ths) : threads(ths) {}
    ~ThreadWrapper() {
        for (auto& it : threads) {
            if (it.joinable())
                it.join();
        }
    }
};
