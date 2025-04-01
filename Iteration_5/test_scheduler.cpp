#include <iostream>
#include <queue>
#include <string>
#include <cassert>

// Define the message types.
enum MessageType {
    ELEVATOR_STATUS,
    FLOOR_REQUEST,
    FAULT,
    SHUTDOWN
};

// Message structure holding a type and a content string.
struct Message {
    MessageType type;
    std::string content;
};

// Scheduler class with a message queue and a shutdown flag.
class Scheduler {
public:
    std::queue<Message> messageQueue;
    bool shutdownInitiated = false;

    // Receives a message; if shutdown is initiated, discard new messages.
    void receiveMessage(const Message& msg) {
        // If shutdown has been initiated, log and discard the message.
        if (shutdownInitiated) {
            std::cout << "[Scheduler] Shutdown initiated, discarding message: "
                      << msg.content << std::endl;
            return;
        }
        // If the message is SHUTDOWN, set the shutdown flag.
        if (msg.type == SHUTDOWN) {
            shutdownInitiated = true;
        }
        // Add the message to the queue.
        messageQueue.push(msg);
    }

    // Process all messages in the queue
    void processAllMessages() {
        while (!messageQueue.empty()) {
            Message msg = messageQueue.front();
            messageQueue.pop();
            // Simulate processing by outputting the message
            std::cout << "[Scheduler] Processing message: " << msg.content << std::endl;
        }
    }
};

int main() {
    Scheduler testScheduler;

    std::cout << "Starting Scheduler Unit Test" << std::endl;

    testScheduler.receiveMessage({ELEVATOR_STATUS, "ELEVATOR_STATUS 1 0 IDLE 0 10 0"});
    testScheduler.receiveMessage({ELEVATOR_STATUS, "ELEVATOR_STATUS 2 0 IDLE 0 10 0"});
    testScheduler.receiveMessage({FLOOR_REQUEST, "FLOOR_REQUEST 1 UP 5 1"});
    testScheduler.receiveMessage({FLOOR_REQUEST, "FLOOR_REQUEST 1 UP 6 1"});
    testScheduler.receiveMessage({FLOOR_REQUEST, "FLOOR_REQUEST 1 UP 6 1"});
    testScheduler.receiveMessage({FLOOR_REQUEST, "FLOOR_REQUEST 1 UP 7 1"});
    testScheduler.receiveMessage({FAULT, "FAULT SENSOR_FAILURE 1"});
    testScheduler.receiveMessage({FLOOR_REQUEST, "FLOOR_REQUEST 1 UP 7 1"});
    testScheduler.receiveMessage({SHUTDOWN, "SHUTDOWN"});
    // This message is sent after shutdown and should be discarded
    testScheduler.receiveMessage({FLOOR_REQUEST, "FLOOR_REQUEST 1 UP 5 1"});

    // Process all messages that were accepted
    testScheduler.processAllMessages();

    // Check that the message queue is empty
    assert(testScheduler.messageQueue.empty());
    std::cout << "Test passed. Message queue is empty." << std::endl;

    return 0;
}
