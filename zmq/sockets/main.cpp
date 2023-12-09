#include <thread>

#include "./../zhelpers.hpp"

struct PORT {
    static constexpr const char* A = "inproc://step2";
    static constexpr const char* B = "inproc://step3";
};


int main () {
	
	zmq::context_t context(1);

    //  Bind to inproc: endpoint, then start upstream thread
    zmq::socket_t receiver (context, ZMQ_PULL);
    receiver.bind(PORT::B);

    std::jthread([&context]() {
        zmq::socket_t receiver (context, ZMQ_PULL);
        receiver.bind(PORT::A);

        std::jthread([&context]() {
            //  Signal downstream to step 2
            zmq::socket_t sender (context, ZMQ_PUSH);
            sender.connect(PORT::A);

            zmq::message_t message(0);
            sender.send (message, 0);

            return (NULL);
        });

        //  Wait for signal
        zmq::message_t messageReceive;
        receiver.recv(&messageReceive, 0);

        //  Signal downstream to step 3
        zmq::socket_t sender (context, ZMQ_PUSH);
        sender.connect(PORT::B);
        zmq::message_t message(0);
        sender.send(message);

        zmq::message_t messageSend(0);
        sender.send (messageSend, 0);

        return (NULL);
    });

    //  Wait for signal
    // see ./../zmq/zhelpers.hpp:102 for getting the message string
    zmq::message_t message;
    receiver.recv(&message, 0);
    
    std::cout << "Test successful!" << std::endl;

    return 0;
}
