/**
 * SYSC3303 - Project Iteration 3
 * Authors: Aj Donald, Jayven Larsen
 * Updated: [Your Date]
 */
#include "Datagram.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <arpa/inet.h>  // for htons()

enum class SchedulerState { IDLE, ASSIGNING, WAITING };

class Scheduler {
public:
    Scheduler() : state(SchedulerState::IDLE), running(true) {}
    ~Scheduler() { running = false; }

    // Start the receiver and processing threads.
    void start() {
        receiverThread = std::thread(&Scheduler::receiveLoop, this);
        processorThread = std::thread(&Scheduler::processLoop, this);
    }

    void join() {
        if (receiverThread.joinable()) receiverThread.join();
        if (processorThread.joinable()) processorThread.join();
    }

private:
    SchedulerState state;
    bool running;

    // Queue and synchronization for received messages.
    std::queue<std::string> messageQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;

    // Threads.
    std::thread receiverThread;
    std::thread processorThread;

    // UDP receive loop.
    void receiveLoop() {
        try {
            DatagramSocket udpSocket(htons(8000));
            std::cout << "[Scheduler] Listening on port 8000...\n";
            while (running) {
                std::vector<uint8_t> buffer(1024);
                DatagramPacket packet(buffer, buffer.size());
                udpSocket.receive(packet);
                std::string msg(buffer.begin(), buffer.begin() + packet.getLength());
                std::cout << "[Scheduler] Received: " << msg << "\n";
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(msg);
                }
                queueCV.notify_one();
            }
        } catch (const std::exception &e) {
            std::cerr << "[Scheduler] Exception in receiver: " << e.what() << "\n";
        }
    }

    // Message processing loop.
    void processLoop() {
        while (running) {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this] { return !messageQueue.empty() || !running; });
            if (!running) break;
            std::string msg = messageQueue.front();
            messageQueue.pop();
            lock.unlock();
            processMessage(msg);
        }
    }

    // Process a received message.
    void processMessage(const std::string &msg) {
        std::istringstream iss(msg);
        std::string type;
        iss >> type;
        if (type == "FLOOR_REQUEST") {
            int floor;
            std::string direction;
            int destination;
            iss >> floor >> direction >> destination;
            std::cout << "[Scheduler] Processing floor request from floor " << floor 
                      << " going " << direction << " to " << destination << "\n";
            // For demonstration, alternate between elevator 1 and 2.
            static int lastAssigned = 0;
            int numElevators = 2; // for demo purposes
            int assignedElevator = (lastAssigned % numElevators) + 1;
            lastAssigned++;

            std::string command = "ASSIGN_ELEVATOR " + std::to_string(assignedElevator) 
                                  + " " + std::to_string(destination);
            int elevatorPort = (assignedElevator == 1) ? 8001 : 8002;
            sendCommand(command, "127.0.0.1", htons(elevatorPort));
        } 
        // Handle Faults
        else if (type == "STUCK_ELEVATOR") {
            int elevator, floor1, floor2;
            iss >> elevator >> floor1 >> floor2;
            std::cout << "[Scheduler] Handling STUCK_ELEVATOR: Elevator " 
                        << elevator << " stuck between floors " << floor1 << " and " << floor2 << "\n";
            // Command to elevator to stop or handle stuck situation
            std::string command = "ELEVATOR_STOP " + std::to_string(elevator);
            int elevatorPort = (elevator == 1) ? 8001 : 8002;
            sendCommand(command, "127.0.0.1", htons(elevatorPort));
        } 
        else if (type == "SENSOR_FAILURE") {
            int floor;
            iss >> floor;
            std::cout << "[Scheduler] Handling SENSOR_FAILURE on floor " << floor << "\n";
            // Inform the floor to handle sensor failure (e.g., reset sensor, wait, etc.)
            std::string command = "SENSOR_RESET " + std::to_string(floor);
            sendCommand(command, "127.0.0.1", htons(8000));  // Send back to the floor
        } 
        else if (type == "DOOR_FAILURE") {
            int floor;
            iss >> floor;
            std::cout << "[Scheduler] Handling DOOR_FAILURE on floor " << floor << "\n";
            // Command to elevator to reset door (or handle door failure)
            std::string command = "DOOR_RESET " + std::to_string(floor);
            sendCommand(command, "127.0.0.1", htons(8001));  // Send to elevator 1
        } else if (type == "ELEVATOR_STATUS") {
            int elevatorId, currentFloor;
            std::string status;
            iss >> elevatorId >> currentFloor >> status;
            std::cout << "[Scheduler] Elevator " << elevatorId << " is at floor " 
                      << currentFloor << " (" << status << ")\n";
            // Additional scheduling logic would be added here.
        } else if (type == "TERMINATE") {
            // Log termination requests without shutting down
            std::cout << "[Scheduler] Received TERMINATE signal. Ignoring and staying active.\n";
        }
    }

    // Send a UDP command message to the specified address/port.
    void sendCommand(const std::string &command, const std::string &address, int port) {
        try {
            DatagramSocket udpSocket;
            std::vector<uint8_t> data(command.begin(), command.end());
            DatagramPacket packet(data, data.size(), inet_addr(address.c_str()), port);
            udpSocket.send(packet);
            std::cout << "[Scheduler] Sent command: " << command 
                      << " to " << address << ":" << ntohs(port) << "\n";
        } catch (const std::exception &e) {
            std::cerr << "[Scheduler] Send error: " << e.what() << "\n";
        }
    }
};

int main() {
    Scheduler scheduler;
    scheduler.start();
    scheduler.join();
    return 0;
}
