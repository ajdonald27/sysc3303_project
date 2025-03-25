#include "Datagram.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <arpa/inet.h>

// Include the Elevator class (from elevator.cpp)
// Make sure elevator.cpp is compiled with -DUNIT_TEST so its main() is excluded.
#include "elevator.cpp"

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
    std::cout << "Starting Elevator Unit Test for Iteration 5 (Capacity and Instrumentation Test)" << std::endl;
    // Create Elevator instance on test port 9005
    Elevator testElevator(1, 0, 9005);
    std::thread elevatorThread([&testElevator](){
        testElevator.start();
        testElevator.join();
    });
    
    // Allow the elevator to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Send 12 ASSIGN_ELEVATOR commands to simulate boarding
    for (int i = 0; i < 12; i++) {
        sendTestCommand("ASSIGN_ELEVATOR 1 5", 9005);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    // Wait for processing
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Send shutdown command
    sendTestCommand("SHUTDOWN", 9005);
    
    elevatorThread.join();
    std::cout << "Elevator Unit Test for Iteration 5 completed. Check output for 'Cannot board, capacity full' message and instrumentation data." << std::endl;
    
    return 0;
}
