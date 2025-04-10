/**
 * SYSC3303 - Project Iteration 5
 * Authors: David Hos, Aj Donald, Jayven Larsen
 * Date: March 23rd, 2025
 *
 * Elevator module: Listens for commands, updates occupancy based on the number of passengers boarding,
 * moves to the target floor, and updates the Scheduler with its status
 */

 #include "Datagram.h"
 #include "Logger.h"
 #include <iostream>
 #include <thread>
 #include <mutex>
 #include <chrono>
 #include <sstream>
 #include <cstdlib>
 #include <arpa/inet.h>
 #include <cmath>
 #include <string>
 #include <vector>
 
 enum class ElevatorState { IDLE, MOVING, ARRIVED, STUCK, DOOR_FAILURE, FULL };
 
 class Elevator {
 public:
     Elevator(int id, int startFloor, int listenPort)
         : elevatorId(id), currentFloor(startFloor), state(ElevatorState::IDLE),
           running(true), port(listenPort), capacity(10), currentOccupancy(0), movementCount(0) {}
  
     ~Elevator() { running = false; }
  
     // Start the UDP listener thread
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
     int port;  // Elevator listens on its own port
     std::mutex stateMutex;
     std::thread listenerThread;
     int capacity;
     int currentOccupancy;
     int movementCount;
  
     // Constants for timing / fault thresholds
     static constexpr int TIME_PER_FLOOR = 5;
     static constexpr int DOOR_OPEN_TIME = 2;
     static constexpr int DOOR_CLOSE_TIME = 2;
     static constexpr int FAULT_THRESHOLD = 2;
  
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
             int id, targetFloor, passengerCount;
             iss >> id >> targetFloor >> passengerCount;
             if (id == elevatorId) {
                 std::lock_guard<std::mutex> lock(stateMutex);
                 if (state == ElevatorState::IDLE) {
                     if (currentOccupancy + passengerCount > capacity) {
                         Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Cannot board " 
                           + std::to_string(passengerCount) + " passenger(s), capacity full. Current occupancy: " + std::to_string(currentOccupancy));
                         sendStatus("FULL");
                     } else {
                         currentOccupancy += passengerCount; // Board the passengers.
                         Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Command received to go to floor " 
                                   + std::to_string(targetFloor) + " with " + std::to_string(passengerCount) + " passenger(s) boarding.");
                         std::thread(&Elevator::moveToFloor, this, targetFloor, passengerCount).detach();
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
         if (type == "SENSOR_RESET") {
             Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Sensor failure detected. Continuing operation.");
             sendStatus("SENSOR_FAILURE");
         } 
         if (type == "DOOR_RESET") {
             Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Door failure detected. Attempting reset...");
             {
                 std::lock_guard<std::mutex> lock(stateMutex);
                 state = ElevatorState::DOOR_FAILURE;
             }
             std::this_thread::sleep_for(std::chrono::seconds(2));
             {
                 std::lock_guard<std::mutex> lock(stateMutex);
                 state = ElevatorState::IDLE; 
             }
             Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Door reset complete.");
             sendStatus("DOOR_FIXED");
         }
     }
  
     // Move to the target floor while updating occupancy
     void moveToFloor(int floor, int passengerCount) {
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
     
                 if (elapsedTime > (std::abs(floor - currentFloor) * TIME_PER_FLOOR + FAULT_THRESHOLD)) {
                     state = ElevatorState::STUCK;
                     Logger::getInstance().log("[Elevator " + std::to_string(elevatorId) + "] Stuck at floor " + std::to_string(currentFloor) + "!");
                     sendStatus("STUCK");
                     return;
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
     
         openDoors();
         closeDoors();
     
         {
             std::lock_guard<std::mutex> lock(stateMutex);
             currentOccupancy -= passengerCount;
             if (currentOccupancy < 0)
                 currentOccupancy = 0;
             state = ElevatorState::IDLE;
         }
         sendStatus("IDLE");
     }
  
     void openDoors() {
         auto startTime = std::chrono::steady_clock::now();
         std::this_thread::sleep_for(std::chrono::seconds(DOOR_OPEN_TIME));
     
        // keeping track of the time 
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
     // Create four elevators on different ports
     Elevator elevator1(1, 0, 8001);
     Elevator elevator2(2, 0, 8002);
     Elevator elevator3(3, 0, 8003);
     Elevator elevator4(4, 0, 8004);
  
     elevator1.start();
     elevator2.start();
     elevator3.start();
     elevator4.start();
  
     elevator1.join();
     elevator2.join();
     elevator3.join();
     elevator4.join();
  
     Logger::getInstance().log("All elevators terminated. Exiting.");
     return 0;
 }
  
 #endif
 