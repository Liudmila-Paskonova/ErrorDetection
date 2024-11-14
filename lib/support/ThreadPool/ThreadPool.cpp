#include <support/ThreadPool/ThreadPool.h>

threadpool::ThreadPool::ThreadPool(size_t numThreads) : tasks(numThreads)
{
    for (size_t i = 0; i < numThreads; ++i) {
        // the i'th worker is doing its job...
        workers.push(size_t(i));

        threads.emplace_back([&, i](const std::stop_token &stop_tok) {
            while (!stop_tok.stop_requested()) {
                // enter the critical section
                tasks[i].semaphore.acquire();
                // while still there're tasks in a queue
                while (waitingTasks.load(std::memory_order_acquire) > 0) {
                    while (auto task = tasks[i].tasks.pop()) {
                        // decrease the number of waiting tasks
                        waitingTasks.fetch_sub(1, std::memory_order_release);
                        // process the given task
                        std::invoke(std::move(task.value()));
                        // decrease the total number of tasks
                        totalLeftTasks.fetch_sub(1, std::memory_order_release);
                    }
                    // if the i'th worker has processed all its tasks, it can do non-processed tasks of another
                    // workers
                    for (size_t j = 1; j < tasks.size(); ++j) {
                        auto workerIdx = (i + j) % tasks.size();
                        if (auto task = tasks[workerIdx].tasks.pop()) {
                            // decrease the number of waiting tasks
                            waitingTasks.fetch_sub(1, std::memory_order_release);
                            // process the given task
                            std::invoke(std::move(task.value()));
                            // decrease the total number of tasks
                            totalLeftTasks.fetch_sub(1, std::memory_order_release);
                            break;
                        }
                    }
                }

                // if all tasks were processed, finish job
                if (totalLeftTasks.load(std::memory_order_acquire) == 0) {
                    doneThreads.store(true, std::memory_order_release);
                    doneThreads.notify_one();
                }
            }
        });
    }
}

threadpool::ThreadPool::~ThreadPool()
{
    // wait until all tasks are processed
    if (totalLeftTasks.load(std::memory_order_acquire) > 0) {
        doneThreads.wait(false);
    }
    // finishing processing: stopping threads and releasing the corresponding semaphores
    for (size_t i = 0; i < threads.size(); ++i) {
        threads[i].request_stop();
        tasks[i].semaphore.release();
        threads[i].join();
    }
}
