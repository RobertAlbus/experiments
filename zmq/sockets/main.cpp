#include <deque>
#include <thread>

#include "./../zhelpers.hpp"

struct PORT {
    static constexpr const char* A = "inproc://port_A";
    static constexpr const char* B = "inproc://port_B";
};


int main () {

    int numWorkers = 8;
    int numIterations = 1000000;
    using namespace std::chrono_literals;
    
    int maj, min, patch;
    zmq::version(&maj, &min, &patch);
    printf("\n%i.%i.%i", maj, min, patch);
    bool shouldRun = true;
    zmq::context_t context(8);

    zmq::socket_t router (context, zmq::socket_type::router);
    router.set(zmq::sockopt::sndhwm, 1000);
    router.set(zmq::sockopt::router_mandatory, 1);
    router.bind(PORT::A);

    std::vector<std::thread> workers;

    for (int i = 0; i < numWorkers; ++i) {
        workers.emplace_back([](void* ctx, void* shouldRun_p, int i, int numIterations) {

            std::string identity = "T" + std::to_string(i);
            printf("\nT%i init", i);
            zmq::context_t &context = *(static_cast<zmq::context_t*>(ctx));
            bool* shouldRun = static_cast<bool*>(shouldRun_p);

            zmq::socket_t request (context, zmq::socket_type::req);
            request.set(zmq::sockopt::routing_id, identity);
            request.set(zmq::sockopt::rcvhwm, 1000);
            request.connect(PORT::A);

            int numReceived = 0;
            int numExpected = numIterations;

            printf("\nT%i start", i);

            while(numReceived < numExpected) {
                zmq::message_t ready(0);
                request.send(ready);
                // printf("\nT%i ready", i);
                zmq::message_t messageReceive;
                request.recv(messageReceive);
                ++numReceived;
                // std::this_thread::sleep_for(0.5s);
                // printf("\nT%i - %i - done: %s", i, numReceived, numReceived < numExpected ? "no" : "yes");
            }
            zmq::message_t done(0);
            request.send(done);
            printf("\nT%i stopping", i);
            
            request.close();
        }, &context, &shouldRun, i+1, numIterations);
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
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

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
    printf("\ntotal time = %ims", duration.count());

    for (auto& worker : workers) {
        worker.join();
    }

    printf("\nTest successful");
    printf("\n");
    router.close();

    return 0;
}
