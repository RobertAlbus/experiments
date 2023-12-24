#include <stdio.h>
#include <chrono>
#include <functional>
#include <numeric>
#include <semaphore>
#include <thread>

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


int main() {
    using namespace std::chrono_literals;

    bool shouldStop = false;
    std::counting_semaphore<> pendingWorkSemaphore(0);
    std::counting_semaphore<> completedWorkSemaphore(0);

    const int largestBatchSize = 100;

    Bins<std::function<void()>> workBins(largestBatchSize);

    std::vector<int> batchSizes {largestBatchSize, 8, 3, 2, 1};
    int batchSize = accumulate(batchSizes.begin(),batchSizes.end(),0);
    int batchCount = 1000;

    float sampleRate = 48000;
    float audioDurationMilliseconds = 1000.f / sampleRate * ((float)batchCount);

    auto workload = 0ns;

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
                // printf("\nworker [bins] %03i | ", i);
                workBins.get(i)();
                doneSemaphore.release();
            }
        });
    }


    auto start = std::chrono::high_resolution_clock::now();
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

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    printf("\ntime spent:     %i ms", duration);
    printf("\naudio duration:  %i ms", (int)audioDurationMilliseconds);

    shouldStop = true;
    for (int i = 0; i < largestBatchSize + 1; ++i) {
        binWorkerPendingWorkSemaphore[i].release();
    }

    printf("\n\n");
}


