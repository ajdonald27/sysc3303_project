/**
 * SYSC3303 - Project Iteration 3
 * Authors: Aj Donald, Jayven Larsen
 * Date: March 2nd, 2025
 */
#include "Datagram.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <arpa/inet.h> 

enum class ElevatorState { IDLE, MOVING, ARRIVED };

class Elevator {
public:
    Elevator(int id, int startFloor, int listenPort)
        : elevatorId(id), currentFloor(startFloor), state(ElevatorState::IDLE),
          running(true), port(listenPort) {}

    ~Elevator() { running = false; }

    // start the thread to listens for UDP
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
    int port;  
    std::mutex stateMutex;
    std::thread listenerThread;

    void listenerLoop() {
        try {
            DatagramSocket udpSocket(htons(port));
            std::cout << "[Elevator " << elevatorId << "] Listening on port " << port << "...\n";
            while (running) {
                std::vector<uint8_t> buffer(1024);
                DatagramPacket packet(buffer, buffer.size());
                udpSocket.receive(packet);
                std::string msg(buffer.begin(), buffer.begin() + packet.getLength());
                std::cout << "[Elevator " << elevatorId << "] Received: " << msg << "\n";
                processCommand(msg);
            }
            std::cout << "[Elevator " << elevatorId << "] Listener loop terminated.\n";
        } catch (const std::exception &e) {
            std::cerr << "[Elevator " << elevatorId << "] Listener exception: " << e.what() << "\n";
        }
    }

    void processCommand(const std::string &command) {
        std::istringstream iss(command);
        std::string type;
        iss >> type;
        // Check for shutdown command.
        if (type == "SHUTDOWN") {
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                std::cout << "[Elevator " << elevatorId << "] Shutdown command received. Terminating listener.\n";
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
                    std::cout << "[Elevator " << elevatorId << "] Command received to go to floor " 
                              << targetFloor << "\n";
                    // launch a thread to simulate movement.
                    std::thread(&Elevator::moveToFloor, this, targetFloor).detach();
                } else {
                    std::cout << "[Elevator " << elevatorId << "] Busy (current state not IDLE). Ignoring command for floor " 
                              << targetFloor << "\n";
                }
            }
        }
    }

    // Simulate moving to a floor.
    void moveToFloor(int floor) {
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = ElevatorState::MOVING;
        }
        while (currentFloor != floor && running) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            {
                std::lock_guard<std::mutex> lock(stateMutex);
                if (currentFloor < floor)
                    currentFloor++;
                else if (currentFloor > floor)
                    currentFloor--;
                std::cout << "[Elevator " << elevatorId << "] Moving... Current floor: " 
                          << currentFloor << "\n";
            }
            sendStatus("MOVING");
        }
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = ElevatorState::ARRIVED;
            std::cout << "[Elevator " << elevatorId << "] Arrived at floor " << currentFloor << "\n";
        }
        sendStatus("ARRIVED");
        {
            std::lock_guard<std::mutex> lock(stateMutex);
            state = ElevatorState::IDLE;
        }
    }

    // Send a status update to the Scheduler.
    void sendStatus(const std::string &status) {
        try {
            DatagramSocket udpSocket;
            std::string update = "ELEVATOR_STATUS " + std::to_string(elevatorId) + " " +
                                 std::to_string(currentFloor) + " " + status;
            std::vector<uint8_t> data(update.begin(), update.end());
            // Change InetAddress::getLocalHost() if you need a different destination IP.
            DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000));
            udpSocket.send(packet);
            std::cout << "[Elevator " << elevatorId << "] Sent status: " << update << "\n";
        } catch (const std::exception &e) {
            std::cerr << "[Elevator " << elevatorId << "] Status send error: " << e.what() << "\n";
        }
    }
};

int main() {
    // Elevator 1 with id 1, starting at floor 0, listening on port 8001.
    // Elevator 2 with id 2, starting at floor 0, listening on port 8002.
    Elevator elevator1(1, 0, 8001);
    Elevator elevator2(2, 0, 8002);

    elevator1.start();
    elevator2.start();

    // Wait for the elevators to finish (they now run indefinitely until a shutdown command is received).
    elevator1.join();
    elevator2.join();

    std::cout << "All elevators terminated. Exiting.\n";
    return 0;
}
