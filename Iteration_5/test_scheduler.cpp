/**
 * SYSC3303 - Project Iteration 5
 * Authors: Sami Kasoua, Adam Sultan
 * Date: March 23rd, 2025
 */
#include "Datagram.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <arpa/inet.h>

#include "scheduler.cpp"

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
    
    // Allow the scheduler to start and listen
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Simulate initial elevator status updates so the scheduler's map is populated.
    sendTestCommand("ELEVATOR_STATUS 1 0 IDLE 0 10 0", 8000);
    sendTestCommand("ELEVATOR_STATUS 2 0 IDLE 0 10 0", 8000);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Send multiple floor requests to test the round-robin load balancing.
    sendTestCommand("FLOOR_REQUEST 1 UP 5", 8000);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sendTestCommand("FLOOR_REQUEST 1 UP 6", 8000);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    sendTestCommand("FLOOR_REQUEST 1 UP 7", 8000);
    
    // Wait for processing and display updates.
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Send shutdown command to end the test.
    sendTestCommand("SHUTDOWN", 8000);
    
    schedulerThread.join();
    std::cout << "Scheduler Unit Test for Iteration 5 completed. Check output for floor request processing, round-robin load balancing, and display updates." << std::endl;
    
    return 0;
}
