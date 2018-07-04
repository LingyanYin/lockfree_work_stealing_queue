#include <memory>
#include <functional>

class FunctionWrapper {
private:
    struct impl_base {
        virtual int call() = 0;
        virtual ~impl_base() {}
    };
    std::unique_ptr<impl_base> impl;

    template <typename F>
    struct impl_type : impl_base {
        F f;
        impl_type(F&& f_) : f(std::move(f_)) {}
        int call() override { return f(); } 
    };
    
public: 

    template <typename F>
    FunctionWrapper(F&& f_) : impl(new impl_type<F>(std::move(f_))) {}

    int operator()() { return impl->call(); }

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
