#include <support/ThreadPool/ThreadSafeQueue.h>
#include <vector>
#include <functional>
#include <thread>
#include <future>

namespace threadpool
{

class ThreadPool
{
    // Task package for each thread
    struct Task {
        // queue of tasks for each worker
        ThreadSafeQueue<std::move_only_function<void()>> tasks{};
        // semaphore to allow only one worker in a critical section
        std::binary_semaphore semaphore{0};
    };

    // vector[numThreads] storing threads corresponding to workers
    std::vector<std::jthread> threads;
    // vector[numThreads] for each worker storing it's Task package, e.g. its own semaphore and tasks queue
    std::vector<Task> tasks;
    // vector with workers' ids
    ThreadSafeQueue<size_t> workers;

    // The counter for tasks waiting in a queue
    std::atomic_int waitingTasks = 0;
    // The counter for tasks left for processing in total
    std::atomic_int totalLeftTasks = 0;
    // The flag which signals if threadpool can be stopped
    std::atomic_bool doneThreads = false;

  public:
    explicit ThreadPool(size_t numThreads = 1);

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

    template <typename Func, typename... Args>
    std::future<void>
    addTask(Func f, Args... args)
    {
        // define the promise
        std::promise<void> promise;

        // get the future
        auto fut = promise.get_future();

        // get worker's idx i from the beginning of the queue
        auto val = workers.pop();
        if (!val.has_value()) {
            return fut;
        }

        auto i = val.value();

        // move this worker to the queue's back
        workers.push(size_t(val.value()));

        // at the beginning
        if (totalLeftTasks == 0) {
            doneThreads.store(false, std::memory_order_release);
        }

        // increase counters
        totalLeftTasks.fetch_add(1, std::memory_order_release);
        waitingTasks.fetch_add(1, std::memory_order_release);

        // add task to the queue
        tasks[i].tasks.push(
            std::move([func = std::move(f), ... largs = std::move(args), promise = std::move(promise)]() mutable {
                func(largs...);
                promise.set_value();
            }));

        // release semaphore
        tasks[i].semaphore.release();

        return fut;
    }
    ~ThreadPool();
};
}; // namespace threadpool
