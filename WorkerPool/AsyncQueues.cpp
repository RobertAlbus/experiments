#include <stdio.h>
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <numeric>
#include <queue>
#include <semaphore>
#include <thread>


#include <sched.h>
#include <unistd.h>

void setRealTimePriority() {
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);

    if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
        perror("sched_setscheduler failed");
        exit(1);
    }
}

void setRealTimePriority(std::jthread& th) {
    int policy = SCHED_FIFO;
    struct sched_param param;
    param.sched_priority = sched_get_priority_max(policy);

    if (pthread_setschedparam(th.native_handle(), policy, &param)) {
        perror("Failed to set thread to real-time priority");
        exit(1);
    }
}

template <typename T>
struct Bins {
    Bins(int size = 10) {
        data.resize(size);
    }

    void set(int bin, T item) {
        data.at(bin) = item;
    }

    T get(int bin) {
        return data.at(bin);
    }

private:
    std::vector<T> data;
};


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
    setRealTimePriority();

    using namespace std::chrono_literals;

    bool shouldStop = false;
    std::counting_semaphore<> pendingWorkSemaphore(0);
    std::counting_semaphore<> completedWorkSemaphore(0);

    LockQueue<std::function<void()>> lockQueue;
    AtomicQueue<std::function<void()>> atomicQueue;

    const int largestBatchSize = 100;

    Bins<std::function<void()>> workBins(largestBatchSize);

    std::vector<int> batchSizes {largestBatchSize, 8, 3, 2, 1};
    int batchSize = accumulate(batchSizes.begin(),batchSizes.end(),0);
    int batchCount = 100;
    int taskCount = batchSize * batchCount;

    float sampleRate = 48000;
    float audioDurationMilliseconds = 1000.f / sampleRate * (float)batchCount;

    int workerCount = 50;
    auto workload = 0ns;


    for (int i = 0; i < taskCount; ++i) {
        auto work = [workload](){
            // printf("task: %i ", i);
            // std::this_thread::sleep_for(workload);
        };
        lockQueue.push(work);
        atomicQueue.push(work);
    }

    // ----------------
    // lock queue
    printf("\n\n----------------");
    printf("\nLOCK QUEUE\n");

    for (int i = 0; i < workerCount; ++i) {
        auto thread = new std::jthread(
                [&lockQueue,
                &shouldStop,
                &pendingWorkSemaphore,
                &completedWorkSemaphore,
                &workload, i]() {

            while (true) {
                pendingWorkSemaphore.acquire();
                if (shouldStop) break;

                // std::this_thread::sleep_for(workload);
                // printf("\nworker [lock] %i | ", i);

                lockQueue.pop()();
                completedWorkSemaphore.release();
            }
        });

        setRealTimePriority(*thread);
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


    for (int i = 0; i < largestBatchSize; ++i) {
        auto thread = new std::jthread(
                [&atomicQueue,
                &shouldStop,
                &pendingWorkSemaphore,
                &completedWorkSemaphore,
                &workload, i]() {

            while (true) {
                pendingWorkSemaphore.acquire();
                if (shouldStop) break;

                // std::this_thread::sleep_for(workload);
                // printf("\nworker [atomic] %i | ", i);

                auto work = atomicQueue.pop();
                if (work) work();
                completedWorkSemaphore.release();
            }
        });

        setRealTimePriority(*thread);
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

    // ----------------
    // work bins

    printf("\n\n----------------");
    printf("\nWORK BINS\n");

    shouldStop = false;

    std::vector<std::jthread> binWorkers;
    std::vector<std::function<void()>> binWork;
    for (int i = 0; i < largestBatchSize; ++i) {
        auto work = [workload, i](){
            // printf("\n[bin] work: %03i ", i);
            // std::this_thread::sleep_for(workload);
        };
        binWork.emplace_back(work);
    }

    std::array binWorkerPendingWorkSemaphore = ([]<std::size_t... I>(std::index_sequence<I...>) { return std::array<std::counting_semaphore<>, sizeof...(I)>{{ ((void) I, std::counting_semaphore(0))... }}; })(std::make_index_sequence<(size_t)largestBatchSize>{});
    std::array binWorkercompletedWorkSemaphore = ([]<std::size_t... I>(std::index_sequence<I...>) { return std::array<std::counting_semaphore<>, sizeof...(I)>{{ ((void) I, std::counting_semaphore(0))... }}; })(std::make_index_sequence<(size_t)largestBatchSize>{});


    for (int i = 0; i < largestBatchSize; ++i) {
        std::counting_semaphore<>& triggerSemaphore = binWorkerPendingWorkSemaphore[i];
        std::counting_semaphore<>& doneSemaphore = binWorkercompletedWorkSemaphore[i];
        binWorkers.emplace_back(
                [&workBins,
                &shouldStop,
                &triggerSemaphore,
                &doneSemaphore,
                i]() {

            while (true) {
                triggerSemaphore.acquire();
                if (shouldStop) break;

                // std::this_thread::sleep_for(0.5s);
                // printf("\nworker [bins] %03i | ", i);

                workBins.get(i)();
                // if (work) work();

                doneSemaphore.release();
            }
        });
        setRealTimePriority(binWorkers[i]);
    }


    start = std::chrono::high_resolution_clock::now();
    for (int batch = 0; batch < batchCount; batch++) {
        // printf("\n\n| batch [bins] %i", batch);
        // printf("\n--------------------------------", batch);
        for (auto size : batchSizes) {
            for (int i = 0; i < size; ++i) {
                // printf("\n[bin] set work %03i", i);
                workBins.set(i, binWork[i]);
            }
            for (int i = 0; i < size; ++i) {
                // printf("\n[bin] trigger %03i", i);
                binWorkerPendingWorkSemaphore[i].release();
            }
            for (int i = 0; i < size; ++i) {
                // printf("\n[bin] waiting %03i", i);
                binWorkercompletedWorkSemaphore[i].acquire();
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
    for (int i = 0; i < largestBatchSize + 1; ++i) {
        binWorkerPendingWorkSemaphore[i].release();
    }

    printf("\n\n");
}


