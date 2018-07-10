#include <memory>
#include <tuple>
#include <utility>

/**
 * previously used a function<void()> as member
 * but it cannot use packaged_task since function is copy-constructed
 * however packaged_task is move-only, changed it to type-erasure class
 */
class FunctionWrapper {
private:
    class ImplBase {
    public:
        virtual void invoke() = 0;
        virtual ~ImplBase() {}
    };

    std::unique_ptr<ImplBase> impl;

    template <typename FunctionType, typename... Args>
    class Impl : public ImplBase {
    public:
        FunctionType f;
        std::tuple<Args...> args;
        Impl(FunctionType&& f_, Args&&... args_) : f(std::move(f_)), args(std::make_tuple(std::move(args_)...)) {}

        void invoke() override { 
            // expland a tuple as it was a parameter pack
            call(std::make_index_sequence<std::tuple_size<std::tuple<Args...>>::value>{});
        }

        template<std::size_t... Indices>
        void call(const std::index_sequence<Indices...>) {
            f(std::get<Indices>(std::forward<std::tuple<Args...>>(args))...);
        }
    };
public: 
    template <typename F, typename... Args>
    FunctionWrapper(F&& f_, Args&&... args_) : impl(new Impl<F, Args...>(std::move(f_), std::move(args_)...)) {}

    void operator()() { impl->invoke(); }

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
