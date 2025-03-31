/**
 * SYSC3303 - Project Iteration 5
 * Authors: Sami Kasoua, Adam Sultan
 * Date: March 23rd, 2025
 *
 * Scheduler Unit Test: Tests floor request processing, round-robin load balancing,
 * and ensures that the internal message queue is empty after shutdown.
 */

 #include "Datagram.h"
 #include <iostream>
 #include <thread>
 #include <chrono>
 #include <vector>
 #include <arpa/inet.h>
 #include <cassert>
 
 // For testing purposes only: expose private members.
 #define private public
 #include "scheduler.cpp"
 #undef private
 
 void sendTestCommand(const std::string &command, int port) {
     try {
         DatagramSocket udpSocket;
         std::vector<uint8_t> data(command.begin(), command.end());
         DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(port));
         udpSocket.send(packet);
     } catch(const std::exception &e) {
         std::cerr << "Error sending test command: " << e.what() << std::endl;
     }
 }
 
 int main() {
     std::cout << "Starting Scheduler Unit Test for Iteration 5" << std::endl;
     Scheduler testScheduler;
     std::thread schedulerThread([&testScheduler](){
         testScheduler.start();
         testScheduler.join();
     });
     
     // Allow the scheduler to start and listen.
     std::this_thread::sleep_for(std::chrono::seconds(1));
     
     // Simulate initial elevator status updates so the scheduler's map is populated.
     sendTestCommand("ELEVATOR_STATUS 1 0 IDLE 0 10 0", 8000);
     sendTestCommand("ELEVATOR_STATUS 2 0 IDLE 0 10 0", 8000);
     std::this_thread::sleep_for(std::chrono::seconds(1));
     
     // Send multiple floor requests (with passenger count 1) to test round-robin load balancing.
     sendTestCommand("FLOOR_REQUEST 1 UP 5 1", 8000);
     std::this_thread::sleep_for(std::chrono::seconds(1));
     sendTestCommand("FLOOR_REQUEST 1 UP 6 1", 8000);
     std::this_thread::sleep_for(std::chrono::seconds(1));
     sendTestCommand("FLOOR_REQUEST 1 UP 7 1", 8000);
     
     // Wait for processing and display updates.
     std::this_thread::sleep_for(std::chrono::seconds(10));
     
     // Send shutdown command to end the test.
     sendTestCommand("SHUTDOWN", 8000);
     
     schedulerThread.join();
     
     // Assert that the scheduler's message queue is empty.
     assert(testScheduler.messageQueue.empty());
     // Assert that the scheduler's running flag is false after shutdown.
     assert(testScheduler.running == false);
     
     std::cout << "Scheduler Unit Test for Iteration 5 completed successfully." << std::endl;
     return 0;
 }
 