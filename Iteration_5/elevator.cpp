/**
 * SYSC3303 - Project Iteration 5
 * Authors: David Hos, Aj Donald, Jayven Larsen
 * Date: March 23rd, 2025
 */
#include "Datagram.h"
#include "Logger.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <arpa/inet.h>  // for htons()
#include <cmath>
#include <string>

enum class ElevatorState { IDLE, MOVING, ARRIVED, STUCK, DOOR_FAILURE, FULL };

class Elevator {
public:
    Elevator(int id, int startFloor, int listenPort)
        : elevatorId(id), currentFloor(startFloor), state(ElevatorState::IDLE),
          running(true), port(listenPort), capacity(10), currentOccupancy(0), movementCount(0) {}
 
    ~Elevator() { running = false; }
 
    // Start the thread that listens for UDP commands.
    void start() {
        listenerThread = std::thread(&Elevator::listenerLoop, this);
    }
 
    void join() {
        if (listenerThread.joinable()) listenerThread.join();
    }
 
private:
    int elevatorId;
    int currentFloor;
    ElevatorState state;
    bool running;
    int port;  // Each elevator listens on its own port.
    std::mutex stateMutex;
    std::thread listenerThread;
    int capacity;
    int currentOccupancy;
    int movementCount;
 
    // Constants for timing and fault thresholds
    static constexpr int TIME_PER_FLOOR = 5; // Time to move between floors (seconds)
    static constexpr int DOOR_OPEN_TIME = 2; // Time to open doors (seconds)
    static constexpr int DOOR_CLOSE_TIME = 2; // Time to close doors (seconds)
    static constexpr int FAULT_THRESHOLD = 2; // Time threshold for faults (seconds)
 
    void listenerLoop() {
        try {
            DatagramSocket udpSocket(htons(port));
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Listening on port " + std::to_string(port) + "...");
            while (running) {
                std::vector<uint8_t> buffer(1024);
                DatagramPacket packet(buffer, buffer.size());
                udpSocket.receive(packet);
                std::string msg(buffer.begin(), buffer.begin() + packet.getLength());
                Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Received: " + msg);
                processCommand(msg);
            }
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Listener loop terminated.");
        } catch (const std::exception &e) {
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Listener exception: " + std::string(e.what()));
        }
    }
 
    void processCommand(const std::string &command) {
        std::istringstream iss(command);
        std::string type;
        iss >> type;
        if (type == "SHUTDOWN") {
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Shutdown command received. Terminating listener.");
                running = false;
            }
            return;
        }
        if (type == "ASSIGN_ELEVATOR") {
            int id, targetFloor;
            iss >> id >> targetFloor;
            if (id == elevatorId) {
                std::lock_guard<std::mutex> lock(stateMutex);
                if (state == ElevatorState::IDLE) {
                    if (currentOccupancy >= capacity) {
                        Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Cannot board, capacity full.");
                        sendStatus("FULL");
                    } else {
                        currentOccupancy++; // passenger boards
                        Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Command received to go to floor " 
                                  + std::to_string(targetFloor));
                        std::thread(&Elevator::moveToFloor, this, targetFloor).detach();
                    }
                } else {
                    Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Busy (current state: " + stateToString(state)
                              +"). Ignoring command for floor " + std::to_string(targetFloor));
                }
            }
        }
        if (type == "ELEVATOR_STOP") {
            int id;
            iss >> id;
            if (id == elevatorId) {
                std::lock_guard<std::mutex> lock(stateMutex);
                state = ElevatorState::STUCK;
                Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Stuck between floors. Notifying scheduler...");
                sendStatus("STUCK");
            }
        } 
        // Handle sensor failure
        if (type == "SENSOR_RESET") {
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Sensor failure detected. Continuing operation.");
            sendStatus("SENSOR_FAILURE");
        } 
        // Handle door failure
        if (type == "DOOR_RESET") {
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Door failure detected. Attempting reset...");
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                state = ElevatorState::DOOR_FAILURE;  // Prevents movement during reset
            }
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate door reset delay
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                state = ElevatorState::IDLE; 
            }
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Door reset complete.");
            sendStatus("DOOR_FIXED");
        }
    }
 
    // Simulate moving to a floor.
    void moveToFloor(int floor) {
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = ElevatorState::MOVING;
        }
        auto startTime = std::chrono::steady_clock::now();
    
        while (currentFloor != floor && running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
    
                // Check for movement timeout (hard fault)
                if (elapsedTime > (std::abs(floor - currentFloor) * TIME_PER_FLOOR + FAULT_THRESHOLD)) {
                    state = ElevatorState::STUCK;
                    Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Stuck at floor " + std::to_string(currentFloor) + "!");
                    sendStatus("STUCK");
                    return; // Stop movement
                }
    
                if (currentFloor < floor) {
                    currentFloor++;
                    movementCount++;
                } else if (currentFloor > floor) {
                    currentFloor--;
                    movementCount++;
                }
                Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Moving... Current floor: " 
                          + std::to_string(currentFloor));
            }
            sendStatus("MOVING");
        }
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = ElevatorState::ARRIVED;
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Arrived at floor " + std::to_string(currentFloor));
        }
        sendStatus("ARRIVED");
    
        // Simulate door operation
        openDoors();
        closeDoors();
    
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            if (currentOccupancy > 0)
                currentOccupancy--; // passenger disembarks
            state = ElevatorState::IDLE;
        }
    }
 
    // Simulate opening doors.
    void openDoors() {
        auto startTime = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(DOOR_OPEN_TIME));
    
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
    
        if (elapsedTime > DOOR_OPEN_TIME + FAULT_THRESHOLD) {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = ElevatorState::DOOR_FAILURE;
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Door stuck open at floor " + std::to_string(currentFloor) + "!");
            sendStatus("DOOR_FAILURE");
        } else {
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Doors opened.");
        }
    }
    
    void closeDoors() {
        auto startTime = std::chrono::steady_clock::now();
        std::this_thread::sleep_for(std::chrono::seconds(DOOR_CLOSE_TIME));
    
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
    
        if (elapsedTime > DOOR_CLOSE_TIME + FAULT_THRESHOLD) {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = ElevatorState::DOOR_FAILURE;
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Door stuck closed at floor " + std::to_string(currentFloor) + "!");
            sendStatus("DOOR_FAILURE");
        } else {
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Doors closed.");
        }
    }
 
    // Send a status update to the Scheduler.
    void sendStatus(const std::string &status) {
        try {
            DatagramSocket udpSocket;
            std::string update = "ELEVATOR_STATUS " + std::to_string(elevatorId) + " " +
                                  std::to_string(currentFloor) + " " + status + " " +
                                  std::to_string(currentOccupancy) + " " + std::to_string(capacity) + " " +
                                  std::to_string(movementCount);
            std::vector<uint8_t> data(update.begin(), update.end());
            DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000));
            udpSocket.send(packet);
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Sent status: " + update);
        } catch (const std::exception &e) {
            Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Status send error: " + std::string(e.what()));
        }
    }
 
    std::string stateToString(ElevatorState state) {
        switch (state) {
            case ElevatorState::IDLE: return "IDLE";
            case ElevatorState::MOVING: return "MOVING";
            case ElevatorState::ARRIVED: return "ARRIVED";
            case ElevatorState::STUCK: return "STUCK";
            case ElevatorState::DOOR_FAILURE: return "DOOR_FAILURE";
            case ElevatorState::FULL: return "FULL";
            default: return "UNKNOWN";
        }
    }
};
 
// Define static constexpr members
constexpr int Elevator::TIME_PER_FLOOR;
constexpr int Elevator::DOOR_OPEN_TIME;
constexpr int Elevator::DOOR_CLOSE_TIME;
constexpr int Elevator::FAULT_THRESHOLD;
 
#ifndef UNIT_TEST
 
int main() {
    // Create four elevators:
    // Elevator 1 with id 1, starting at floor 0, listening on port 8001.
    // Elevator 2 with id 2, starting at floor 0, listening on port 8002.
    // Elevator 3 with id 3, starting at floor 0, listening on port 8003.
    // Elevator 4 with id 4, starting at floor 0, listening on port 8004.
    Elevator elevator1(1, 0, 8001);
    Elevator elevator2(2, 0, 8002);
    Elevator elevator3(3, 0, 8003);
    Elevator elevator4(4, 0, 8004);
 
    elevator1.start();
    elevator2.start();
    elevator3.start();
    elevator4.start();
 
    // Wait for the elevators to finish.
    elevator1.join();
    elevator2.join();
    elevator3.join();
    elevator4.join();
 
    Logger::getInstance().log("All elevators terminated. Exiting.");
    return 0;
}
 
#endif
