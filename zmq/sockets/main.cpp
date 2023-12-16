#include <cmath>
#include <deque>
#include <thread>

#include "./../zhelpers.hpp"

/*

## ZeroMQ worker thread wake-up

This POC uses a Request-Reply event loop implemented over inproc sockets.
* worker sends a ready-signal and wait for wake-up
* main thread receives ready-signals from workers asynchronously
* main thread dispatches work-signals from workers asynchronously
* worker receives work-signal and executes

--------------------------------
audio time:        2083 ms
number of workers: 8
buffer size:       1 samples
buffer fb latency: 00.0208 ms
graph depth:       8

 multi-threaded: 24722 ms
single-threaded: 7 ms

real time budget consumed
 multi-threaded: 1186.6559%
single-threaded: 00.3360%

single-threaded could perform: 3531.71x more work in the same time
--------------------------------


--------------------------------
audio time:        4166 ms
number of workers: 8
buffer size:       2 samples
buffer fb latency: 00.0417 ms
graph depth:       8

 multi-threaded: 22296 ms
single-threaded: 15 ms

real time budget consumed
 multi-threaded: 535.1039%
single-threaded: 00.3600%

single-threaded could perform: 1486.40x more work in the same time
--------------------------------


--------------------------------
audio time:        8333 ms
number of workers: 8
buffer size:       4 samples
buffer fb latency: 00.0833 ms
graph depth:       8

 multi-threaded: 22719 ms
single-threaded: 32 ms

real time budget consumed
 multi-threaded: 272.6280%
single-threaded: 00.3840%

single-threaded could perform: 709.97x more work in the same time
--------------------------------


--------------------------------
audio time:        16666 ms
number of workers: 8
buffer size:       8 samples
buffer fb latency: 00.1667 ms
graph depth:       8

 multi-threaded: 23163 ms
single-threaded: 62 ms

real time budget consumed
 multi-threaded: 138.9780%
single-threaded: 00.3720%

single-threaded could perform: 373.60x more work in the same time
--------------------------------


--------------------------------
audio time:        33333 ms
number of workers: 8
buffer size:       16 samples
buffer fb latency: 00.3333 ms
graph depth:       8

 multi-threaded: 26574 ms
single-threaded: 120 ms

real time budget consumed
 multi-threaded: 79.7220%
single-threaded: 00.3600%

single-threaded could perform: 221.45x more work in the same time
--------------------------------


--------------------------------
audio time:        66666 ms
number of workers: 8
buffer size:       32 samples
buffer fb latency: 00.6667 ms
graph depth:       8

 multi-threaded: 25913 ms
single-threaded: 233 ms

real time budget consumed
 multi-threaded: 38.8695%
single-threaded: 00.3495%

single-threaded could perform: 111.21x more work in the same time
--------------------------------


--------------------------------
audio time:        133333 ms
number of workers: 8
buffer size:       64 samples
buffer fb latency: 01.3333 ms
graph depth:       8

 multi-threaded: 24409 ms
single-threaded: 432 ms

real time budget consumed
 multi-threaded: 18.3067%
single-threaded: 00.3240%

single-threaded could perform: 56.50x more work in the same time
--------------------------------

--------------------------------
audio time:        266666 ms
number of workers: 8
buffer size:       128 samples
buffer fb latency: 02.6667 ms
graph depth:       8

 multi-threaded: 22319 ms
single-threaded: 874 ms

real time budget consumed
 multi-threaded: 08.3696%
single-threaded: 00.3277%

single-threaded could perform: 25.54x more work in the same time
--------------------------------


--------------------------------
audio time:        533333 ms
number of workers: 8
buffer size:       256 samples
buffer fb latency: 05.3333 ms
graph depth:       8

 multi-threaded: 23710 ms
single-threaded: 1470 ms

real time budget consumed
 multi-threaded: 04.4456%
single-threaded: 00.2756%

single-threaded could perform: 16.13x more work in the same time
--------------------------------
*/

struct PORT {
    static constexpr const char* A = "inproc://port_A";
    static constexpr const char* B = "inproc://port_B";
};


int main () {

    int numWorkers = 8;
    int graphDepth = 8;
    int numIterations = 100000 * graphDepth;
    int workerBufferSize = 256;
    float sampleRate = 48000.f;
    float ms_per_s = 1000.f;
    using namespace std::chrono_literals;
    
    int maj, min, patch;
    zmq::version(&maj, &min, &patch);
    printf("\n%i.%i.%i", maj, min, patch);

    zmq::context_t context(8);

    zmq::socket_t router (context, zmq::socket_type::router);
    router.set(zmq::sockopt::sndhwm, 1000);
    router.set(zmq::sockopt::router_mandatory, 1);
    router.bind(PORT::A);

    std::vector<std::thread> workers;

    for (int i = 0; i < numWorkers; ++i) {
        workers.emplace_back([](void* ctx, int i, int numIterations, int workerBufferSize) {
            

            std::string identity = "T" + std::to_string(i);
            printf("\nT%i init", i);
            zmq::context_t &context = *(static_cast<zmq::context_t*>(ctx));

            zmq::socket_t request (context, zmq::socket_type::req);
            request.set(zmq::sockopt::routing_id, identity);
            request.set(zmq::sockopt::rcvhwm, 1000);
            request.connect(PORT::A);

            int numReceived = 0;
            int numExpected = numIterations;

            printf("\nT%i start", i);
            
            float workValue = 3.00045;
            while(numReceived < numExpected) {
                zmq::message_t ready(0);
                request.send(ready);
                // printf("\nT%i ready", i);
                zmq::message_t messageReceive;
                request.recv(messageReceive);
                ++numReceived;
                int i = 0;
                while (i<workerBufferSize) {
                    ++i;
                    // simulating work
                    workValue *= workValue;
                    if(workValue > 1000000) workValue = 3.00045;
                }

                // std::this_thread::sleep_for(0.5s);
                // printf("\nT%i - %i - done: %s", i, numReceived, numReceived < numExpected ? "no" : "yes");
            }
            zmq::message_t done(0);
            request.send(done);
            printf("\nT%i stopping", i);
            printf(" | work value %f", workValue); // print the work value so it isn't optimized away by the compiler

            
            request.close();
        }, &context, i+1, numIterations, workerBufferSize);
        printf("\nT%i started", i);
    }

    // std::this_thread::sleep_for(1s);

    std::deque<std::string> readyWorkers {};
    // Listen for workers coming online
    for (int i = 0; i < numWorkers; ++i) {
        zmq::message_t identity;
        router.recv(identity);

        readyWorkers.emplace_back(identity.to_string());

        zmq::message_t delim;
        router.recv(delim);

        zmq::message_t msg;
        router.recv(msg);

        printf("\nT0 greeted %i threads", i + 1);
    }
    printf("\nT0 all threads online");
    for (auto identity : readyWorkers) {
        printf("\n- %s", identity.c_str());
    }
    printf("\n");

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numIterations; ++i){
        // printf("\n----------------");
        // printf("\nT0 iteration %i", i+1);
        for (int i = 0; i < numWorkers; ++i) {
            // printf("\nT0 replying to %s", readyWorkers.front().c_str());
            zmq::message_t identity(readyWorkers.front());
            readyWorkers.pop_front();

            router.send(identity, zmq::send_flags::sndmore);
            zmq::message_t delim(std::string(""));
            router.send(delim, zmq::send_flags::sndmore);
            zmq::message_t msg(std::string(""));
            router.send(msg);
        }
        for (int i = 0; i < numWorkers; ++i) {
            zmq::message_t identity;
            router.recv(identity);
            readyWorkers.emplace_back(identity.to_string());

            zmq::message_t delim;
            router.recv(delim);
            zmq::message_t msg;
            router.recv(msg);

            // printf("\nT0 ready from %s", identity.to_string().c_str());
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_multi_thread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // take workers offline
    for (int i = 0; i < numWorkers; ++i) {
        // printf("\nT0 replying to %s", readyWorkers.front().c_str());
        zmq::message_t identity(readyWorkers.front());
        readyWorkers.pop_front();

        router.send(identity, zmq::send_flags::sndmore);
        zmq::message_t delim(std::string(""));
        router.send(delim, zmq::send_flags::sndmore);
        zmq::message_t msg(std::string(""));
        router.send(msg);
    }

    // std::this_thread::sleep_for(5s);

    for (auto& worker : workers) {
        worker.join();
    }
    router.close();

    ////////
    // SIGNLE THREADED COMPARISON

    start = std::chrono::high_resolution_clock::now();
    int i = 0;
    float workValue = 3.00045;
    int framesToProcess = numIterations * workerBufferSize * numWorkers;
    while (i<framesToProcess) {
        ++i;
        // simulating work
        workValue *= workValue;
        if(workValue > 1000000) workValue = 3.00045;
    }
    end = std::chrono::high_resolution_clock::now();
    auto duration_single_thread = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("\nwork value %f", workValue); // print the work value so it isn't optimized away by the compiler

    int mt_ms = duration_multi_thread.count();
    int st_ms = duration_single_thread.count();
    float mt_ms_f = (float) mt_ms;
    float st_ms_f = (float) st_ms;
    float audio_ms_f = ms_per_s / sampleRate * (float)numIterations * (float)workerBufferSize / (float)graphDepth;
    int audio_ms = (int) audio_ms_f;
    float feedback_latency_ms = ms_per_s / sampleRate * (float)workerBufferSize;

    float mt_real_time_budget_percent_consumed = mt_ms_f / audio_ms_f * 100.f;
    float st_real_time_budget_percent_consumed = st_ms_f / audio_ms_f * 100.f;

    printf("\n");
    printf("\n--------------------------------");
    printf("\naudio time:        %i ms", audio_ms);
    printf("\nnumber of workers: %i", numWorkers);
    printf("\nbuffer size:       %i samples", workerBufferSize);
    printf("\nbuffer fb latency: %07.4f ms", feedback_latency_ms);
    printf("\ngraph depth:       %i", graphDepth);
    printf("\n");
    printf("\n multi-threaded: %i ms", mt_ms);
    printf("\nsingle-threaded: %i ms", st_ms);
    printf("\n");
    printf("\nreal time budget consumed");
    printf("\n multi-threaded: %07.4f%%", mt_real_time_budget_percent_consumed);
    printf("\nsingle-threaded: %07.4f%%", st_real_time_budget_percent_consumed);
    printf("\n");
    printf("\nsingle-threaded could perform: %.2fx more work in the same time",
        mt_real_time_budget_percent_consumed /st_real_time_budget_percent_consumed);
    printf("\n--------------------------------");
    printf("\n");

    return 0;
}
