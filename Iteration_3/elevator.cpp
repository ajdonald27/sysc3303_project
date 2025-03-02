// elevator.cpp
#include "Datagram.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <arpa/inet.h>  // for htons()

enum class ElevatorState { IDLE, MOVING, ARRIVED };

class Elevator {
public:
    Elevator(int id, int startFloor)
        : elevatorId(id), currentFloor(startFloor), state(ElevatorState::IDLE), running(true) {}

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
    std::mutex stateMutex;
    std::thread listenerThread;

    void listenerLoop() {
        try {
            DatagramSocket udpSocket(htons(8001));
            std::cout << "[Elevator " << elevatorId << "] Listening on port 8001...\n";
            while (running) {
                std::vector<uint8_t> buffer(1024);
                DatagramPacket packet(buffer, buffer.size());
                udpSocket.receive(packet);
                std::string msg(buffer.begin(), buffer.begin() + packet.getLength());
                std::cout << "[Elevator " << elevatorId << "] Received: " << msg << "\n";
                processCommand(msg);
            }
        } catch (const std::exception &e) {
            std::cerr << "[Elevator " << elevatorId << "] Listener exception: " << e.what() << "\n";
        }
    }

    void processCommand(const std::string &command) {
        std::istringstream iss(command);
        std::string type;
        iss >> type;
        if (type == "ASSIGN_ELEVATOR") {
            int id, targetFloor;
            iss >> id >> targetFloor;
            if (id == elevatorId) {
                std::cout << "[Elevator " << elevatorId << "] Command received to go to floor " 
                          << targetFloor << "\n";
                // Launch a state machine thread to handle movement.
                std::thread(&Elevator::moveToFloor, this, targetFloor).detach();
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
                currentFloor += (currentFloor < floor ? 1 : -1);
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
            DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000));
            udpSocket.send(packet);
            std::cout << "[Elevator " << elevatorId << "] Sent status: " << update << "\n";
        } catch (const std::exception &e) {
            std::cerr << "[Elevator " << elevatorId << "] Status send error: " << e.what() << "\n";
        }
    }
};

int main() {
    // Create an elevator with ID 1 starting at floor 0.
    Elevator elevator(1, 0);
    elevator.start();
    elevator.join();
    return 0;
}
