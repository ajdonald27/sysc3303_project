/**
 * SYSC3303 - Project Iteration 4
 * Authors:David Hos, Aj Donald, Jayven Larsen
 * Date: March 17th, 2025
 */
#include "Datagram.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <arpa/inet.h>  // for htons()

enum class FloorState { IDLE, REQUESTING };

class Floor {
public:
    Floor(int floorNum) 
        : floorNumber(floorNum), state(FloorState::IDLE), running(true) {}

    ~Floor() { running = false; }

    // Start the thread that processes the trace file.
    void start() {
        simulationThread = std::thread(&Floor::processTraceFile, this, "trace.txt");
    }

    void join() {
        if (simulationThread.joinable()) simulationThread.join();
    }

private:
    int floorNumber;
    FloorState state;
    bool running;
    std::thread simulationThread;

    // Read and process each request in the trace file.
    void processTraceFile(const std::string &filename) {
        std::ifstream infile(filename);
        if (!infile) {
            std::cerr << "[Floor " << floorNumber << "] Error opening trace file.\n";
            return;
        }
        std::string line;
        
        while (std::getline(infile, line)) {
            if (line.empty() || line[0] == '#') continue; // Skip comments and empty lines
            // Expected format: "hh:mm:ss.mmm floor direction destination fault"
            std::istringstream iss(line);
            std::string timestamp, direction, faultType = "None";
            int requestFloor, destination;

            if (!(iss >> timestamp >> requestFloor >> direction >> destination >> faultType)) {
                std::cerr << "[Floor " << floorNumber << "] Malformed line in trace file.\n";
                continue;
            }
            iss >> faultType;
            // Only process requests for this floor.
            if (requestFloor == floorNumber) {
                if (faultType != "None") {
                    handleFault(faultType, requestFloor);
                } else {
                    sendRequest(destination, direction);
                }
            }
            // Simulate real-time by waiting a short period between requests.
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
        std::cout << "[Floor " << floorNumber << "] Finished processing trace file.\n";
    }

    void handleFault(const std::string &faultType, int faultFloor) {
        std::string faultMessage = "FAULT " + faultType + " " + std::to_string(faultFloor);
        std::cout << "[Floor " << floorNumber << "] Injecting fault: " << faultMessage << "\n";
    
        // Send fault information to the scheduler
        try {
            DatagramSocket udpSocket;
            std::vector<uint8_t> data(faultMessage.begin(), faultMessage.end());
            DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000)); // Assuming 8000 is the scheduler's port
            udpSocket.send(packet);
            std::cout << "[Floor " << floorNumber << "] Sent fault: " << faultMessage << "\n";
        } catch (const std::exception &e) {
            std::cerr << "[Floor " << floorNumber << "] Send error: " << e.what() << "\n";
        }
        
    }
    

    // Send a floor request to the Scheduler.
    void sendRequest(int destinationFloor, const std::string &direction) {
        state = FloorState::REQUESTING;
        std::string request = "FLOOR_REQUEST " + std::to_string(floorNumber) + " " +
                              direction + " " + std::to_string(destinationFloor);
        try {
            DatagramSocket udpSocket;
            std::vector<uint8_t> data(request.begin(), request.end());
            DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000));
            udpSocket.send(packet);
            std::cout << "[Floor " << floorNumber << "] Sent request: " << request << "\n";
        } catch (const std::exception &e) {
            std::cerr << "[Floor " << floorNumber << "] Send error: " << e.what() << "\n";
        }
        state = FloorState::IDLE;
    }
};

int main() {
    // For demonstration, simulate floor 1.
    Floor floor(1);
    floor.start();
    floor.join();
    std::cout << "[Floor 1] Exiting program.\n";
    return 0;
}
