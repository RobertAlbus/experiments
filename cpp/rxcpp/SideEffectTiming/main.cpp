
#include "rxcpp/rx.hpp"
#include "rxcpp/rx-test.hpp"

#include <chrono>
#include <iostream> 
#include <memory>
#include <numeric>
#include <semaphore>


int main() {
    using namespace std::chrono_literals;
    int sampleRate = 48000;
    int tenMinutes = sampleRate * 60 * 10;
    int oneMinute = sampleRate * 60 * 1;
    int duration = oneMinute / 10;

    int numWaiters = 10;

    std::counting_semaphore waitForWorkers(0);
    std::counting_semaphore waitForChanger(0);
    std::counting_semaphore<> *changer = &waitForChanger;
    printf("[thread 0] Start task\n");

    auto manualTicker = rxcpp::subjects::behavior<int>(0);
    auto clock = rxcpp::observable<>::range(1, duration, rxcpp::observe_on_new_thread());
    auto start = std::chrono::high_resolution_clock::now();

    auto dependersScheduler = rxcpp::schedulers::make_new_thread();
    

    float outputValue = 0;
    for (int i = 0; i < numWaiters; ++i) {
        manualTicker.get_observable().
            // observe_on(rxcpp::observe_on_one_worker(dependersScheduler)).
            observe_on(rxcpp::observe_on_event_loop()).

            subscribe(
                [&outputValue, &waitForChanger, &waitForWorkers, i](int v) {
                    waitForChanger.acquire();
                    // printf("\nemitter: %02f | %02i | %02i", outputValue, i, v);
                    waitForWorkers.release();
                },
                [start, &waitForWorkers](){
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    printf("\ntime spent: %i ms", duration);
                    printf("\n[emitter] OnCompleted");
                    waitForWorkers.release();
                });
    }


    manualTicker.get_observable().
        observe_on(rxcpp::observe_on_new_thread()).

        subscribe(
            [&outputValue, &waitForChanger, &waitForWorkers, numWaiters](int v) {
                if (v % 2 == 0) {
                    // printf("\nchanger: %02f | %02i", (float)v, v);
                    outputValue = (float)v;
                }
                for (int i = 0; i < numWaiters; ++i) {
                    waitForChanger.release();
                }
                waitForWorkers.release();
            },
            [start, &waitForWorkers](){
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                printf("\ntime spent: %i ms", duration);
                printf("\n[changer] OnCompleted");
                waitForWorkers.release();
            });
    printf("\n[thread 0] Finish task");

    int totalObservers = numWaiters + 1;
    for (int count = 1; count < duration; ++count) {
        manualTicker.get_subscriber().on_next(count);
        if (count % 1000 == 0) printf("\n%05i", count);
        for (int i = 0; i < totalObservers; ++i) {
            waitForWorkers.acquire();
        }
    }

    manualTicker.get_subscriber().on_completed();

    // wait for oncomplete
    for (int i = 0; i < totalObservers; ++i) {
        waitForWorkers.acquire();
    }
    printf("\n");
}
