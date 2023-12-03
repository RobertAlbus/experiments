
#include "rxcpp/rx.hpp"
#include "rxcpp/rx-test.hpp"


#include <chrono>
#include <iostream> 
#include <memory>
#include <numeric>
#include <semaphore>
#include <thread> 
#include <unistd.h>

#include "WavetableOscillatorMono.h"


std::string get_pid() {
    std::stringstream s;
    s << std::this_thread::get_id();
    return s.str();
}

int main() {
    int sampleRate = 4800;
    int oneHour = sampleRate * 60 * 60;
    int duration = oneHour;
    int numWorkers = 100;

    std::counting_semaphore waitForWorkers(0);

    printf("//! [threaded range sample]\n");
    printf("[thread 0] Start task\n");

    std::shared_ptr<std::vector<float>> waveTable = std::make_shared<std::vector<float>>(Sine(128));
    std::vector<WavetableOscillatorMono> oscillators;
    for (int i = 0; i < numWorkers; i++){
        oscillators.emplace_back(sampleRate, waveTable);
        oscillators[i].freq(1000 + i);
    }

    auto clock = rxcpp::observable<>::range(1, duration, rxcpp::observe_on_new_thread());
    auto start = std::chrono::high_resolution_clock::now();


    for (int i = 0; i < numWorkers; i++){
        WavetableOscillatorMono& oscillator = oscillators[i];
        clock.
            // observe_on(rxcpp::observe_on_new_thread()).
            subscribe(
                [&oscillator](int v) {
                    float result = oscillator.process();
                },
                [start, i, &waitForWorkers](){
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                    waitForWorkers.release();
                    printf("\ntime spent: %i ms", duration);
                    printf("\n[thread %i] OnCompleted", i);
                });
    }
    printf("\n[thread 0] Finish task");
    printf("\n//! [threaded range sample]");

    for (int i = 0; i < numWorkers; ++i) {
        waitForWorkers.acquire();
    }
}
