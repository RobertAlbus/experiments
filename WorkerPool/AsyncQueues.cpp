#include <stdio.h>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <semaphore>
#include <thread>

template <typename T>
class ThreadSafeQueue {
public:
    virtual void push(T&& item) = 0;
    virtual T pop() = 0;
    virtual bool empty() const = 0;
};

template <typename T>
class LockQueue : public ThreadSafeQueue<T> {
private:
    std::queue<T> queue;
    std::mutex push_mutex;
    std::mutex pop_mutex;

public:
    void push(T&& item) override {
        std::lock_guard<std::mutex> lock(push_mutex);
        queue.push(std::move(item));
    }

    T pop() override {
        std::lock_guard<std::mutex> lock(pop_mutex);
        T item = std::move(queue.front());
        queue.pop();
        return std::move(item);
    }

    bool empty() const override {
        return queue.empty();
    }
};

template <typename T>
class AtomicQueue : public ThreadSafeQueue<T> {
public:
    struct Task {
        T data;
        std::shared_ptr<Task> next = nullptr;
    };

    void push(T&& item) override {
        std::shared_ptr<Task> oldTailTask = tail.load(std::memory_order_acquire);
        if (oldTailTask != nullptr) {
            std::shared_ptr<Task> newTask = std::make_shared<Task>(item);
            oldTailTask.get()->next = newTask;
            tail.exchange(newTask, std::memory_order_release);
        } else {
            std::shared_ptr<Task> newTask = std::make_shared<Task>(item);
            head.exchange(newTask, std::memory_order_acq_rel);
            tail.exchange(newTask, std::memory_order_release);
        }
    }

    T pop() override {
        std::shared_ptr<Task> oldHeadTask = head.load(std::memory_order_acquire);
        // if (oldHeadTask.get() == nullptr) {
        //     printf("\n-------- error: no task. --------");
        // }
        return std::move(head.exchange(oldHeadTask->next, std::memory_order_release)->data);
    }

    bool empty() const override {
        return head.load(std::memory_order_acq_rel).get() == nullptr;
    }
private:
    std::atomic<std::shared_ptr<Task>> head;
    std::atomic<std::shared_ptr<Task>> tail;
};



int main() {
    using namespace std::chrono_literals;

    bool shouldStop = false;
    std::counting_semaphore<> pendingWorkSemaphore(0);
    std::counting_semaphore<> completedWorkSemaphore(0);

    LockQueue<std::function<void()>> lockQueue;
    AtomicQueue<std::function<void()>> atomicQueue;

    std::vector<int> batchSizes {100, 8, 3, 2, 1};
    int batchSize = accumulate(batchSizes.begin(),batchSizes.end(),0);
    int batchCount = 100;
    int taskCount = batchSize * batchCount;

    float sampleRate = 48000;
    float audioDurationMilliseconds = 1.f / sampleRate * batchCount * 1000;

    int workerCount = 50;
    auto workload = 1ns;

    for (int i = 0; i < taskCount; ++i) {
        float result_L = 1;
        float result_R = 1;
        auto work = [i, &workload, &result_L, &result_R](){
            // printf("task: %i ", i);
            std::this_thread::sleep_for(workload);
        };
        lockQueue.push(work);
        atomicQueue.push(work);
    }

    // ----------------
    // lock queue
    printf("\n\n----------------");
    printf("\nLOCK QUEUE\n");

    for (int i = 0; i < workerCount; ++i) {
        new std::jthread(
                [&lockQueue,
                &shouldStop,
                &pendingWorkSemaphore,
                &completedWorkSemaphore,
                &workload, i]() {

            while (true) {
                pendingWorkSemaphore.acquire();
                if (shouldStop) break;

                std::this_thread::sleep_for(workload);
                // printf("\nworker [lock] %i | ", i);

                lockQueue.pop()();
                completedWorkSemaphore.release();
            }
        });
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int batch = 0; batch < batchCount; batch++) {
        // printf("\n\n| batch [lock] %i", batch);
        // printf("\n--------------------------------", batch);
        for (auto size : batchSizes) {
            for (int i = 0; i < size; ++i) {
                pendingWorkSemaphore.release();
            }
        }
        for (auto size : batchSizes) {
            for (int i = 0; i < size; ++i) {
                completedWorkSemaphore.acquire();
            }
        }

    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    auto workPerformed = std::chrono::duration_cast<std::chrono::milliseconds>(taskCount * workload).count();

    printf("\nwork performed: %i ms", workPerformed);
    printf("\ntime spent:     %i ms", duration);
    printf("\naudio duration:  %i ms", (int)audioDurationMilliseconds);

    shouldStop = true;
    for (int i = 0; i < workerCount + 0; ++i) {
        pendingWorkSemaphore.release();
    }

    printf("\n\n");

    std::this_thread::sleep_for(1s);

    // ----------------
    // atomic queue

    printf("\n\n----------------");
    printf("\nATOMIC QUEUE\n");

    shouldStop = false;


    for (int i = 0; i < workerCount; ++i) {
        new std::jthread(
                [&atomicQueue,
                &shouldStop,
                &pendingWorkSemaphore,
                &completedWorkSemaphore,
                &workload, i]() {

            while (true) {
                pendingWorkSemaphore.acquire();
                if (shouldStop) break;

                std::this_thread::sleep_for(workload);
                // printf("\nworker [atomic] %i | ", i);

                auto work = atomicQueue.pop();
                if (work) work();
                completedWorkSemaphore.release();
            }
        });
    }

    start = std::chrono::high_resolution_clock::now();
    for (int batch = 0; batch < batchCount; batch++) {
        // printf("\n\n| batch [atomic] %i", batch);
        // printf("\n--------------------------------", batch);
        for (auto size : batchSizes) {
            for (int i = 0; i < size; ++i) {
                pendingWorkSemaphore.release();
            }
        }
        for (auto size : batchSizes) {
            for (int i = 0; i < size; ++i) {
                completedWorkSemaphore.acquire();
            }
        }
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    workPerformed = std::chrono::duration_cast<std::chrono::milliseconds>(taskCount * workload).count();

    printf("\nwork performed: %i ms", workPerformed);
    printf("\ntime spent:     %i ms", duration);
    printf("\naudio duration:  %i ms", (int)audioDurationMilliseconds);

    shouldStop = true;
    for (int i = 0; i < workerCount + 1; ++i) {
        pendingWorkSemaphore.release();
    }

    printf("\n\n");
}


