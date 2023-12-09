//
//  Multithreaded relay in C++
//

#include "./../zhelpers.hpp"

//  Step 1 pushes one message to step 2

void *step1 (void *arg) {
	
	zmq::context_t * context = static_cast<zmq::context_t*>(arg);
	
	//  Signal downstream to step 2
	zmq::socket_t sender (*context, ZMQ_PUSH);
	sender.connect("inproc://step2");

    zmq::message_t message(0);
    sender.send (message, 0);

	return (NULL);
}

//  Step 2 relays the signal to step 3

void *step2 (void *arg) {

	zmq::context_t * context = static_cast<zmq::context_t*>(arg);
	
    //  Bind to inproc: endpoint, then start upstream thread
	zmq::socket_t receiver (*context, ZMQ_PULL);
    receiver.bind("inproc://step2");

    pthread_t thread;
    pthread_create (&thread, NULL, step1, context);

    //  Wait for signal
    zmq::message_t messageReceive;
    receiver.recv(&messageReceive, 0);

    //  Signal downstream to step 3
    zmq::socket_t sender (*context, ZMQ_PUSH);
    sender.connect("inproc://step3");
    s_send (sender, std::string(""));

    zmq::message_t messageSend(0);
    sender.send (messageSend, 0);

    return (NULL);
}

//  Main program starts steps 1 and 2 and acts as step 3

int main () {
	
	zmq::context_t context(1);

    //  Bind to inproc: endpoint, then start upstream thread
    zmq::socket_t receiver (context, ZMQ_PULL);
    receiver.bind("inproc://step3");

    pthread_t thread;
    pthread_create (&thread, NULL, step2, &context);

    //  Wait for signal
    // see ./../zmq/zhelpers.hpp:102 for getting the message string
    zmq::message_t message;
    receiver.recv(&message, 0);
    
    std::cout << "Test successful!" << std::endl;

    return 0;
}
