/**
 * SYSC3303 - Project Iteration 5
 * Authors: David Hos, Aj Donald, Jayven Larsen
 * Date: March 23rd, 2025
 *
 * Scheduler receives floor requests and assigns elevators using round-robin scheduling.
 */

 #include "Datagram.h"
 #include "Logger.h"
 #include <fstream>
 #include <sstream>
 #include <thread>
 #include <queue>
 #include <mutex>
 #include <condition_variable>
 #include <vector>
 #include <unordered_map>
 #include <chrono>
 #include <arpa/inet.h>  // for htons()
 #include <string>
 #include <cmath>
 
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
     Scheduler() 
         : state(SchedulerState::IDLE), running(true), displayRunning(true), nextElevator(1) {}
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
         Logger::getInstance().log("[Scheduler] Simulation time: " + std::to_string(elapsed) +
                                   " seconds. Total movements: " + std::to_string(totalMovements));
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
     
     // Round-robin scheduling for 4 elevators.
     int nextElevator;  // Starts with elevator 1.
  
     void receiveLoop() {
         try {
             DatagramSocket udpSocket(htons(8000));
             Logger::getInstance().log("[Scheduler] Listening on port 8000...");
             while (running) {
                 std::vector<uint8_t> buffer(1024);
                 DatagramPacket packet(buffer, buffer.size());
                 udpSocket.receive(packet);
                 std::string msg(buffer.begin(), buffer.begin() + packet.getLength());
                 Logger::getInstance().log("[Scheduler] Received: " + msg);
                 {
                     std::lock_guard<std::mutex> lock(queueMutex);
                     messageQueue.push(msg);
                 }
                 queueCV.notify_one();
             }
         } catch (const std::exception &e) {
             Logger::getInstance().log("[Scheduler] Exception in receiver: " + std::string(e.what()));
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
             int passengerCount;  // Number of passengers boarding.
             iss >> floor >> direction >> destination >> passengerCount;
             Logger::getInstance().log("[Scheduler] Processing floor request from floor " + std::to_string(floor) +
                       " going " + direction + " to " + std::to_string(destination) +
                       " with " + std::to_string(passengerCount) + " passenger(s).");
             int numElevators = 4; // Now 4 elevators
             int assignedElevator = -1;
             {
                 std::lock_guard<std::mutex> lock(statusMutex);
                 for (int i = 0; i < numElevators; i++) {
                     int elevatorId = nextElevator;
                     if (elevatorStatusMap.find(elevatorId) == elevatorStatusMap.end() || 
                         elevatorStatusMap[elevatorId].occupancy < elevatorStatusMap[elevatorId].capacity) {
                         assignedElevator = elevatorId;
                         break;
                     }
                     nextElevator = (nextElevator % numElevators) + 1;
                 }
                 if (assignedElevator != -1) {
                     nextElevator = (assignedElevator % numElevators) + 1;
                 }
             }
             if (assignedElevator == -1) {
                 Logger::getInstance().log("[Scheduler] All elevators full. Requeuing request.");
                 {
                     std::lock_guard<std::mutex> lock(queueMutex);
                     messageQueue.push(msg);
                 }
                 std::this_thread::sleep_for(std::chrono::seconds(2));
                 queueCV.notify_one();
             } else {
                 Logger::getInstance().log("[Scheduler] Round-robin selection: Elevator " + std::to_string(assignedElevator) +
                           " assigned for floor request from floor " + std::to_string(floor) +
                           " to " + std::to_string(destination) + " with " + std::to_string(passengerCount) +
                           " passenger(s). Next elevator pointer set to " + std::to_string(nextElevator) + ".");
                 std::string command = "ASSIGN_ELEVATOR " + std::to_string(assignedElevator) 
                                       + " " + std::to_string(destination)
                                       + " " + std::to_string(passengerCount);
                 int elevatorPort = 0;
                 if (assignedElevator == 1) elevatorPort = 8001;
                 else if (assignedElevator == 2) elevatorPort = 8002;
                 else if (assignedElevator == 3) elevatorPort = 8003;
                 else if (assignedElevator == 4) elevatorPort = 8004;
                 sendCommand(command, "127.0.0.1", htons(elevatorPort));
             }
         }
         else if (type == "FAULT") {
             std::string faultType;
             int floor;
             iss >> faultType >> floor;
             Logger::getInstance().log("[Scheduler] Handling fault: " + faultType + " on floor " + std::to_string(floor));
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
             Logger::getInstance().log("[Scheduler] Elevator " + std::to_string(elevatorId) + " is at floor " +
                       std::to_string(currentFloor) + " (" + status + ") Occupancy: " + std::to_string(occupancy) +
                       "/" + std::to_string(capacity) + " Movements: " + std::to_string(movementCount));
         }
         else if (type == "SHUTDOWN") {
             Logger::getInstance().log("[Scheduler] Shutdown command received.");
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
             Logger::getInstance().log("[Scheduler] Sent command: " + command + " to " + address + ":" + std::to_string(ntohs(port)));
         } catch (const std::exception &e) {
             Logger::getInstance().log("[Scheduler] Send error: " + std::string(e.what()));
         }
     }
  
     void displayLoop() {
         while (displayRunning) {
             {
                 std::lock_guard<std::mutex> lock(statusMutex);
                 Logger::getInstance().log("----- Elevator Status -----");
                 for (auto &entry : elevatorStatusMap) {
                     Logger::getInstance().log("Elevator " + std::to_string(entry.first) + " - Floor: " + std::to_string(entry.second.floor) +
                               ", Occupancy: " + std::to_string(entry.second.occupancy) + "/" + std::to_string(entry.second.capacity) +
                               ", Status: " + entry.second.status + ", Movements: " + std::to_string(entry.second.movementCount));
                 }
                 Logger::getInstance().log("---------------------------");
             }
             // Slow display update (every 3 seconds)
             std::this_thread::sleep_for(std::chrono::seconds(3));
         }
     }
 };
  
 #ifndef UNIT_TEST
  
 int main() {
     Scheduler scheduler;
     scheduler.start();
     scheduler.join();
     return 0;
 }
  
 #endif
 