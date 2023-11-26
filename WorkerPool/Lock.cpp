#include <stdio.h>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>


template <typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    std::mutex push_mutex;
    std::mutex pop_mutex;

public:
    void push(T&& item) {
        std::lock_guard<std::mutex> lock(push_mutex);
        queue.push(std::move(item));
    }

    T pop() {
        std::lock_guard<std::mutex> lock(pop_mutex);
        T item = std::move(queue.front());
        queue.pop();
        return std::move(item);
    }

    bool empty() const {
        return queue.empty();
    }
};


int main() {
    using namespace std::chrono_literals;

    bool shouldStop = false;
    std::counting_semaphore<> pendingWorkSemaphore(0);
    std::counting_semaphore<> completedWorkSemaphore(0);

    ThreadSafeQueue<std::function<void()>> tasks;

    int batchSize = 100;
    int batchCount = 100;
    int taskCount = batchSize * batchCount;

    int workerCount = 10;
    auto workload = 0.01ms;

    for (int i = 0; i < taskCount; ++i) {
        tasks.push([i, &workload](){
            // printf("task: %i ", i);
            std::this_thread::sleep_for(workload);

        });
    }

    for (int i = 0; i < workerCount; ++i) {
        new std::jthread(
                [&tasks,
                &shouldStop,
                &pendingWorkSemaphore,
                &completedWorkSemaphore,
                &workload, i]() {

            while (true) {
                pendingWorkSemaphore.acquire();
                if (shouldStop) break;

                // std::this_thread::sleep_for(workload);

                // printf("\nworker %i | ", i);
                tasks.pop()();

                completedWorkSemaphore.release();
            }
        });
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int batch = 0; batch < batchCount; batch++) {
        // printf("\n\n| batch %i", batch);
        // printf("\n--------------------------------", batch);
        for (int i = 0; i < batchSize; ++i) {
            pendingWorkSemaphore.release();
        }

        for (int i = 0; i < batchSize; ++i) {
            completedWorkSemaphore.acquire();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    auto workPerformed = std::chrono::duration_cast<std::chrono::milliseconds>(taskCount * workload).count();

    printf("\nwork performed: %i ms", workPerformed);
    printf("\ntime spent:     %i ms", duration);

    shouldStop = true;
    for (int i = 0; i < taskCount + 0; ++i) {
        pendingWorkSemaphore.release();
    }

    printf("\n\n");
}


