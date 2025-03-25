#include "Datagram.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <arpa/inet.h>

// Include the Scheduler class (from scheduler.cpp)
// Make sure scheduler.cpp is compiled with -DUNIT_TEST to exclude its main() function.
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
    
    // Allow the scheduler to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Send a floor request command
    sendTestCommand("FLOOR_REQUEST 1 UP 5", 8000);
    
    // Wait for processing and display updates
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Send shutdown command
    sendTestCommand("SHUTDOWN", 8000);
    
    schedulerThread.join();
    std::cout << "Scheduler Unit Test for Iteration 5 completed. Check output for floor request processing and display updates." << std::endl;
    
    return 0;
}
