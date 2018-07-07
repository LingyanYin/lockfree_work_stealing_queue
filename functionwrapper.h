#include <memory>
#include <functional>

class FunctionWrapper {
private:
    std::function<void()> callback;
public: 
    template <typename F, typename... Args>
    FunctionWrapper(F&& f_, Args&&... args_) : callback([f_, args_...]() { f_(args_...); }) {}

    void operator()() { callback(); }

    FunctionWrapper() = default;
    FunctionWrapper(FunctionWrapper&& other) : callback(std::move(other.callback)) {}
    FunctionWrapper& operator=(FunctionWrapper&& other) {
        callback = std::move(other.callback);
        return *this;
    }

    FunctionWrapper(const FunctionWrapper&) = delete;
    FunctionWrapper(FunctionWrapper&) = delete;
    FunctionWrapper& operator=(const FunctionWrapper&) = delete;
};
