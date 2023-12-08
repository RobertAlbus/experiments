/*
a continuation of WorkerPool/WorkerDelegation.cpp and RX related experiments
wherein I attempt to speed up the processing of a graph by parallelizing

Function stream
----
time spent:     14282 ms
audio duration:  2083 ms

in-thread called in series
time spent:       230 ms
audio duration:  2083 ms

- Function streams are real-time performant for sufficiently small graphs.
- function streams can still take 75x the time to complete the same workload though.

Work bin stream
----
time spent:     48344 ms
audio duration:  2083 ms

*/

#include "rxcpp/rx.hpp"
#include "rxcpp/rx-test.hpp"

#include <stdio.h>
#include <chrono>
#include <functional>
#include <numeric>
#include <semaphore>
#include <thread>


int main() {
    using namespace std::chrono_literals;

    std::counting_semaphore<> completedWorkSemaphore(0);

    const int largestBatchSize = 1000;

    std::vector<int> batchSizes {largestBatchSize, 8, 40, 5, 1};
    int cumulativeBatchSize = accumulate(batchSizes.begin(),batchSizes.end(),0);
    int batchCount = 100000;

    float sampleRate = 48000;
    float audioDurationMilliseconds = 1000.f / sampleRate * ((float)batchCount);

    auto workload = 0ns;

    std::vector<std::function<void()>> processFns;
    for (int i = 0; i < cumulativeBatchSize; ++i) {
        auto work = [workload, i](){
            // printf("\n[bin] work: %03i ", i);
            // std::this_thread::sleep_for(workload);
        };
        processFns.emplace_back(work);
    }

    auto workStream = rxcpp::subjects::behavior<
        std::vector<std::reference_wrapper<std::function<void()>>>
    >({});

    std::chrono::_V2::system_clock::time_point end;
    for (int i = 0; i < largestBatchSize; ++i) {
        workStream.get_observable().subscribe(
            [&completedWorkSemaphore, i](std::vector<std::reference_wrapper<std::function<void()>>> workloadVector) {
                if (i < workloadVector.size()) {
                    workloadVector.at(i).get()();
                    completedWorkSemaphore.release();
                }
            },
            [&completedWorkSemaphore, &end]() {
                end = std::chrono::high_resolution_clock::now();
                completedWorkSemaphore.release();
            }
        );
    }


    auto start = std::chrono::high_resolution_clock::now();
    for (int batch = 0; batch < batchCount; batch++) {
        // printf("\n\n| batch [bins] %i", batch);
        // printf("\n--------------------------------", batch);
        int offset = 0;
        for (auto size : batchSizes) {
            std::vector<std::reference_wrapper<std::function<void()>>> subvector;
            for (int i = 0; i < size; ++i) {
                // printf("\n[bin] set work %03i", offset + i);
                subvector.emplace_back(processFns[offset + i]);
            }
            workStream.get_subscriber().on_next(subvector);
            for (int i = 0; i < size; ++i) {
                // printf("\n[bin] waiting  %03i", offset + i);
                completedWorkSemaphore.acquire();
                // printf("\n[bin] waited");
            }
            // printf("\n[bin] done loops");
            offset += size;
        }
    }

    workStream.get_subscriber().on_completed();
    for (int i = 0; i < largestBatchSize; ++i) {
        completedWorkSemaphore.acquire();
    }

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    printf("\ntime spent:     %i ms", duration);
    printf("\naudio duration:  %i ms", (int)audioDurationMilliseconds);

    start = std::chrono::high_resolution_clock::now();
    for (int batch = 0; batch < batchCount; batch++) {
        // printf("\n\n| batch [bins] %i", batch);
        // printf("\n--------------------------------", batch);
        int offset = 0;
        for (auto size : batchSizes) {
            for (int i = 0; i < size; ++i) {
                // printf("\n[bin] set work %03i", i);
                processFns[offset + i]();
            }
            offset += size;
        }
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    printf("\ntime spent:     %i ms", duration);
    printf("\naudio duration:  %i ms", (int)audioDurationMilliseconds);

    printf("\n\n");
}


