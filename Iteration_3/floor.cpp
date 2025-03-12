/**
 * SYSC3303 - Project Iteration 3
 * Authors: Aj Donald, Jayven Larsen
 * March 2nd, 2025
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
        while (running && std::getline(infile, line)) {
            // Expected format: "hh:mm:ss.mmm floor direction destination"
            std::istringstream iss(line);
            std::string timestamp, direction;
            int requestFloor, destination;
            if (!(iss >> timestamp >> requestFloor >> direction >> destination)) {
                std::cerr << "[Floor " << floorNumber << "] Malformed line in trace file.\n";
                continue;
            }
            // Only process requests for this floor.
            if (requestFloor == floorNumber) {
                sendRequest(destination, direction);
            }
            // Simulate real-time by waiting a short period between requests.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "[Floor " << floorNumber << "] Finished processing trace file.\n";
    }

    // Send a floor request to the Scheduler.
    void sendRequest(int destinationFloor, const std::string &direction) {
        state = FloorState::REQUESTING;
        std::string request = "FLOOR_REQUEST " + std::to_string(floorNumber) + " " +
                              direction + " " + std::to_string(destinationFloor);
        try {
            DatagramSocket udpSocket;
            std::vector<uint8_t> data(request.begin(), request.end());
            DatagramPacket packet(data, data.size(),InetAddress::getLocalHost(), htons(8000));
            udpSocket.send(packet);
            std::cout << "[Floor " << floorNumber << "] Sent request: " << request << "\n";
        } catch (const std::exception &e) {
            std::cerr << "[Floor " << floorNumber << "] Send error: " << e.what() << "\n";
        }
        state = FloorState::IDLE;
    }
};

int main() {
    
    Floor floor(1);
    floor.start();
    floor.join();
    std::cout << "[Floor 1] Exiting program.\n";
    return 0;
}
