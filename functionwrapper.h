#include <memory>
#include <functional>

class FunctionWrapper {
private:
    class ImplInterface {
    public:
        virtual int invoke() = 0;
        virtual ~ImplInterface() {}
    };

    std::unique_ptr<ImplInterface> impl;

    template <typename F>
    class Impl : public ImplInterface {
    private:
        F f;
    public:
        Impl(F&& f_) : f(std::move(f_)) {}
        int invoke() override { return f(); } 
    };
    
public: 
    template <typename F>
    FunctionWrapper(F&& f_) : impl(new Impl<F>(std::move(f_))) {}

    int operator()() { return impl->invoke(); }

    FunctionWrapper() = default;
    FunctionWrapper(FunctionWrapper&& other) : impl(std::move(other.impl)) {}
    FunctionWrapper& operator=(FunctionWrapper&& other) {
        impl = std::move(other.impl);
        return *this;
    }

    FunctionWrapper(const FunctionWrapper&) = delete;
    FunctionWrapper(FunctionWrapper&) = delete;
    FunctionWrapper& operator=(const FunctionWrapper&) = delete;
};
