#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>

namespace
{
class ThreadPool
{
  public:
    using ItemType = std::function<void()>;

  private:
    std::queue<ItemType> tasks;
    std::vector<std::thread> workers;
    std::mutex m;
    std::condition_variable c;
    std::atomic<bool> stop_requested;

  public:
    ThreadPool(std::size_t nr_threads = std::thread::hardware_concurrency()) : stop_requested{false}
    {

        for (size_t i = 0; i < nr_threads; i++) {
            workers.emplace_back([this] {
                while (true) {
                    ItemType task;
                    {
                        std::unique_lock<std::mutex> lk(m);
                        while (tasks.empty() && !stop_requested) {
                            c.wait(lk);
                        }
                        if (stop_requested && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lk(m);
            stop_requested = true;
        }
        c.notify_all();
        for (auto &worker : workers) {
            worker.join();
        }
    }

    void
    push(ItemType task)
    {
        {
            std::lock_guard lk(m);
            tasks.emplace(std::move(task));
        }
        c.notify_one();
    }
};
}; // namespace
