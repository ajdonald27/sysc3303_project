/**
 * SYSC3303 - Project Iteration 4
 * Authors:David Hos, Aj Donald, Jayven Larsen
 * Date: March 17th, 2025
 */
#include "Datagram.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <arpa/inet.h>  // for htons()

enum class SchedulerState { IDLE, ASSIGNING, WAITING };

struct ElevatorInfo {
    int floor;
    int occupancy;
    int capacity;
    int movementCount;
    std::string status;
};

class Scheduler {
public:
    Scheduler() : state(SchedulerState::IDLE), running(true), displayRunning(true) {}
    ~Scheduler() { running = false; displayRunning = false; }
 
    // Start the receiver, processor, and display threads.
    void start() {
        startTime = std::chrono::steady_clock::now();
        receiverThread = std::thread(&Scheduler::receiveLoop, this);
        processorThread = std::thread(&Scheduler::processLoop, this);
        displayThread = std::thread(&Scheduler::displayLoop, this);
    }
 
    void join() {
        if (receiverThread.joinable()) receiverThread.join();
        if (processorThread.joinable()) processorThread.join();
        if (displayThread.joinable()) displayThread.join();
        auto endTime = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();
        int totalMovements = 0;
        {
            std::lock_guard<std::mutex> lock(statusMutex);
            for (auto &entry : elevatorStatusMap) {
                totalMovements += entry.second.movementCount;
            }
        }
        std::cout << "[Scheduler] Simulation time: " << elapsed << " seconds. Total movements: " << totalMovements << "\n";
    }
 
private:
    SchedulerState state;
    bool running;
    bool displayRunning;
 
    std::queue<std::string> messageQueue;
    std::mutex queueMutex;
    std::condition_variable queueCV;
 
    std::thread receiverThread;
    std::thread processorThread;
    std::thread displayThread;
 
    std::unordered_map<int, ElevatorInfo> elevatorStatusMap;
    std::mutex statusMutex;
 
    std::chrono::steady_clock::time_point startTime;
 
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
            int numElevators = 2;
            int assignedElevator = -1;
            {
                std::lock_guard<std::mutex> lock(statusMutex);
                for (int i = 1; i <= numElevators; i++) {
                    if (elevatorStatusMap.find(i) != elevatorStatusMap.end()) {
                        ElevatorInfo info = elevatorStatusMap[i];
                        if (info.occupancy < info.capacity) {
                            assignedElevator = i;
                            break;
                        }
                    } else {
                        assignedElevator = i;
                        break;
                    }
                }
            }
            if (assignedElevator == -1) {
                std::cout << "[Scheduler] All elevators full. Requeuing request.\n";
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    messageQueue.push(msg);
                }
                std::this_thread::sleep_for(std::chrono::seconds(2));
                queueCV.notify_one();
            } else {
                std::string command = "ASSIGN_ELEVATOR " + std::to_string(assignedElevator) 
                                      + " " + std::to_string(destination);
                int elevatorPort = (assignedElevator == 1) ? 8001 : 8002;
                sendCommand(command, "127.0.0.1", htons(elevatorPort));
            }
        }
        else if (type == "FAULT") {
            std::string faultType;
            int floor;
            iss >> faultType >> floor;
            std::cout << "[Scheduler] Handling fault: " << faultType << " on floor " << floor << "\n";
            std::string command = "FAULT " + faultType + " " + std::to_string(floor);
            int elevatorPort = (floor % 2 == 1) ? 8001 : 8002;
            sendCommand(command, "127.0.0.1", htons(elevatorPort));
        } 
        else if (type == "ELEVATOR_STATUS") {
            int elevatorId, currentFloor, occupancy, capacity, movementCount;
            std::string status;
            iss >> elevatorId >> currentFloor >> status >> occupancy >> capacity >> movementCount;
            {
                std::lock_guard<std::mutex> lock(statusMutex);
                elevatorStatusMap[elevatorId] = {currentFloor, occupancy, capacity, movementCount, status};
            }
            std::cout << "[Scheduler] Elevator " << elevatorId << " is at floor " << currentFloor 
                      << " (" << status << ") Occupancy: " << occupancy << "/" << capacity 
                      << " Movements: " << movementCount << "\n";
        }
        else if (type == "SHUTDOWN") {
            std::cout << "[Scheduler] Shutdown command received.\n";
            running = false;
            displayRunning = false;
        }
    }
 
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
 
    void displayLoop() {
        std::unordered_map<int, ElevatorInfo> lastPrintedMap;
        while (displayRunning) {
            bool changed = false;
            {
                std::lock_guard<std::mutex> lock(statusMutex);
                if (lastPrintedMap.size() != elevatorStatusMap.size()) {
                    changed = true;
                } else {
                    for (auto &entry : elevatorStatusMap) {
                        int id = entry.first;
                        const ElevatorInfo &currentInfo = entry.second;
                        if (lastPrintedMap.find(id) == lastPrintedMap.end() ||
                            lastPrintedMap[id].floor != currentInfo.floor ||
                            lastPrintedMap[id].occupancy != currentInfo.occupancy ||
                            lastPrintedMap[id].capacity != currentInfo.capacity ||
                            lastPrintedMap[id].movementCount != currentInfo.movementCount ||
                            lastPrintedMap[id].status != currentInfo.status) {
                            changed = true;
                            break;
                        }
                    }
                }
                if (changed) {
                    lastPrintedMap = elevatorStatusMap;
                    std::cout << "----- Elevator Status -----\n";
                    for (auto &entry : elevatorStatusMap) {
                        std::cout << "Elevator " << entry.first << " - Floor: " << entry.second.floor 
                                  << ", Occupancy: " << entry.second.occupancy << "/" << entry.second.capacity 
                                  << ", Status: " << entry.second.status 
                                  << ", Movements: " << entry.second.movementCount << "\n";
                    }
                    std::cout << "---------------------------\n";
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};
 
int main() {
    Scheduler scheduler;
    scheduler.start();
    scheduler.join();
    return 0;
}
