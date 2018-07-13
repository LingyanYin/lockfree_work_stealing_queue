#include <mutex>
#include <condition_variable>
#include <memory>

// for debug
#include <cassert>

template <typename T>
class ThreadSafeQueue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::mutex lock_head;
    std::mutex lock_tail;
    std::unique_ptr<node> head;
    node* tail;
    std::condition_variable data_cond;
    
    // grab lock_tail and get the tail
    node* get_tail() {
        std::lock_guard<std::mutex> tail_lk(lock_tail);
        return tail;
    }

    // pop head without grabing lock_head
    std::unique_ptr<node> pop_head() {
        std::unique_ptr<node> old_head = std::move(head);
        assert(old_head->next != nullptr);
        head = std::move(old_head->next);
        assert(head != nullptr);
        return old_head;
    }

    // wait to get lock_head
    std::unique_lock<std::mutex> wait_for_data() {
        std::unique_lock<std::mutex> head_lk(lock_head);
        data_cond.wait(head_lk, [&] { return head.get() != get_tail(); });
        return std::move(head_lk);
    }

    // wait, grab lock_head and pop head
    std::unique_ptr<node> wait_pop_head() {
        std::unique_lock<std::mutex> head_lk(&ThreadSafeQueue::wait_for_data);
        return pop_head();
    }

    // wait, grab lock_head and pop head
    std::unique_ptr<node> wait_pop_head(T& value) {
        std::unique_lock<std::mutex> head_lk(&ThreadSafeQueue::wait_for_data);
        value = std::move(*(head->data));
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head() {
        std::lock_guard<std::mutex> head_lk(lock_head);
        if (head.get() == get_tail())
            return std::unique_ptr<node>{};
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T& value) {
        std::lock_guard<std::mutex> head_lk(lock_head);
        if (head.get() == get_tail())
            return std::unique_ptr<node>{};
        // assert(head != nullptr);
        value = std::move(*(head->data));
        return pop_head();
    }

public:
    // the queue is empty, head and tail points to the same object
    ThreadSafeQueue() : head(new node), tail(head.get()) {}

    ThreadSafeQueue(const ThreadSafeQueue& other) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue& other) = delete;

    void push(T value);

    std::shared_ptr<T> wait_and_pop() {
        std::unique_ptr<node> old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T& value) {
        std::unique_ptr<node> old_head = wait_pop_head(value);
    }

    std::shared_ptr<T> try_pop() {
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>{};
    }

    bool try_pop(T& value) {
        std::unique_ptr<node> old_head = try_pop_head(value);
        return old_head ? true : false;
    }

    bool empty() {
        std::lock_guard<std::mutex> head_lk(lock_head);
        return head.get() == get_tail();
    }
};

template <typename T>
void ThreadSafeQueue<T>::push(T value) {
    std::shared_ptr<T> new_value(std::make_shared<T>(std::move(value)));
    std::unique_ptr<node> p(new node);
    {
        std::lock_guard<std::mutex> tail_lk(lock_tail);
        tail->data = new_value;
        node* const new_tail = p.get();
        tail->next = std::move(p);
        tail = new_tail;
    }
    data_cond.notify_one();
}
