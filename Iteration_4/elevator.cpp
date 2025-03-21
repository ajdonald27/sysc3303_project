/**
 * SYSC3303 - Project Iteration 4
 * Authors:David Hos, Aj Donald, Jayven Larsen
 * Date: March 17th, 2025
 */
 #include "Datagram.h"
 #include <iostream>
 #include <thread>
 #include <mutex>
 #include <chrono>
 #include <vector>
 #include <sstream>
 #include <cstdlib>
 #include <arpa/inet.h>  // for htons()
 
 enum class ElevatorState { IDLE, MOVING, ARRIVED, STUCK, DOOR_FAILURE };
 
 class Elevator {
 public:
     Elevator(int id, int startFloor, int listenPort)
         : elevatorId(id), currentFloor(startFloor), state(ElevatorState::IDLE),
           running(true), port(listenPort) {}
 
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
 
     // Constants for timing and fault thresholds
     static constexpr int TIME_PER_FLOOR = 5; // Time to move between floors (seconds)
     static constexpr int DOOR_OPEN_TIME = 2; // Time to open doors (seconds)
     static constexpr int DOOR_CLOSE_TIME = 2; // Time to close doors (seconds)
     static constexpr int FAULT_THRESHOLD = 2; // Time threshold for faults (seconds)
 
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
                     // Launch a thread to simulate movement.
                     std::thread(&Elevator::moveToFloor, this, targetFloor).detach();
                 } else {
                     std::cout << "[Elevator " << elevatorId << "] Busy (current state: " << stateToString(state) <<"). Ignoring command for floor " 
                               << targetFloor << "\n";
                 }
             }
         }
         if (type == "ELEVATOR_STOP") {
             int id;
             iss >> id;
             if (id == elevatorId) {
                 std::lock_guard<std::mutex> lock(stateMutex);
                 state = ElevatorState::STUCK;
                 std::cout << "[Elevator " << elevatorId << "] Stuck between floors. Notifying scheduler...\n";
                 sendStatus("STUCK");
             }
         } 
         // Handle sensor failure
         if (type == "SENSOR_RESET") {
             std::cout << "[Elevator " << elevatorId << "] Sensor failure detected. Continuing operation.\n";
             sendStatus("SENSOR_FAILURE");
         } 
         // Handle door failure
         if (type == "DOOR_RESET") {
             std::cout << "[Elevator " << elevatorId << "] Door failure detected. Attempting reset...\n";
             {
                 std::lock_guard<std::mutex> lock(stateMutex);
                 state = ElevatorState::DOOR_FAILURE;  // Prevents movement during reset
             }
             std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate door reset delay
             {
                 std::lock_guard<std::mutex> lock(stateMutex);
                 state = ElevatorState::IDLE; 
             }
             std::cout << "[Elevator " << elevatorId << "] Door reset complete.\n";
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
                if (elapsedTime > (abs(floor - currentFloor) * TIME_PER_FLOOR + FAULT_THRESHOLD)) {
                    state = ElevatorState::STUCK;
                    std::cout << "[Elevator " << elevatorId << "] Stuck at floor " << currentFloor << "!\n";
                    sendStatus("STUCK");
                    return; // Stop movement
                }
    
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
    
        // Simulate door operation
        openDoors();
        closeDoors();
    
        {
            std::lock_guard<std::mutex> lock(stateMutex);
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
            std::cout << "[Elevator " << elevatorId << "] Door stuck open at floor " << currentFloor << "!\n";
            sendStatus("DOOR_FAILURE");
        } else {
            std::cout << "[Elevator " << elevatorId << "] Doors opened.\n";
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
            std::cout << "[Elevator " << elevatorId << "] Door stuck closed at floor " << currentFloor << "!\n";
            sendStatus("DOOR_FAILURE");
        } else {
            std::cout << "[Elevator " << elevatorId << "] Doors closed.\n";
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
 
     std::string stateToString(ElevatorState state) {
         switch (state) {
             case ElevatorState::IDLE: return "IDLE";
             case ElevatorState::MOVING: return "MOVING";
             case ElevatorState::ARRIVED: return "ARRIVED";
             case ElevatorState::STUCK: return "STUCK";
             case ElevatorState::DOOR_FAILURE: return "DOOR_FAILURE";
             default: return "UNKNOWN";
         }
     }
 };
 
// Define static constexpr members
constexpr int Elevator::TIME_PER_FLOOR;
constexpr int Elevator::DOOR_OPEN_TIME;
constexpr int Elevator::DOOR_CLOSE_TIME;
constexpr int Elevator::FAULT_THRESHOLD;

 int main() {
     // Create two elevators:
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