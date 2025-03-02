// scheduler.cpp

/**
 * SYSC3303 - Project Iteration 3
 * Authors: Aj Donald, Jayven Larsen
 * Date: March 2nd, 2025
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
            // For demonstration, assign elevator 1 for all requests.
            std::string command = "ASSIGN_ELEVATOR 1 " + std::to_string(floor);
            sendCommand(command, "127.0.0.1", htons(8001));
        } else if (type == "ELEVATOR_STATUS") {
            int elevatorId, currentFloor;
            std::string status;
            iss >> elevatorId >> currentFloor >> status;
            std::cout << "[Scheduler] Elevator " << elevatorId << " is at floor " 
                      << currentFloor << " (" << status << ")\n";
            // Additional scheduling logic would be added here.
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
                      << " to " << address << ":" << port << "\n";
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
