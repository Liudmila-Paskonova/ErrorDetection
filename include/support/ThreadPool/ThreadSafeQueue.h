#include <mutex>
#include <queue>
#include <optional>

namespace threadpool
{
template <typename T> class ThreadSafeQueue
{
  public:
    ThreadSafeQueue() = default;

    ~ThreadSafeQueue() = default;

    void
    push(T &&value)
    {
        std::lock_guard lk(m);
        q.push(std::move(value));
    }

    std::optional<T>
    pop()
    {
        std::lock_guard lk(m);
        if (q.empty()) {
            return std::nullopt;
        }

        auto front = std::move(q.front());
        q.pop();
        return front;
    }

    bool
    empty() const
    {
        std::lock_guard lk(m);
        return q.empty();
    }

    size_t
    size() const
    {
        std::lock_guard lk(m);
        return q.size();
    }

  private:
    std::queue<T> q{};
    mutable std::mutex m{};
};
}; // namespace threadpool
