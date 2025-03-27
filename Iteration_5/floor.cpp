/**
 * SYSC3303 - Project Iteration 5
 * Authors: David Hos, Aj Donald, Jayven Larsen
 * Date: March 23rd, 2025
 */
#include "Datagram.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <arpa/inet.h>  // for htons()
#include <string>

enum class FloorState { IDLE, REQUESTING };

class Floor {
public:
    Floor(int floorNum) 
        : floorNumber(floorNum), state(FloorState::IDLE), running(true) {}
 
    ~Floor() { running = false; }
 
    // Start the thread that processes the trace file.
    void start(const std::string &traceFile) {
        simulationThread = std::thread(&Floor::processTraceFile, this, traceFile);
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
            Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Error opening trace file: " + filename);
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
                Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Malformed line in trace file.");
                continue;
            }
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
        Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Finished processing trace file.");
    }
 
    void handleFault(const std::string &faultType, int faultFloor) {
        std::string faultMessage = "FAULT " + faultType + " " + std::to_string(faultFloor);
        Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Injecting fault: " + faultMessage);
    
        // Send fault information to the scheduler
        try {
            DatagramSocket udpSocket;
            std::vector<uint8_t> data(faultMessage.begin(), faultMessage.end());
            DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000)); // Assuming 8000 is the scheduler's port
            udpSocket.send(packet);
            Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Sent fault: " + faultMessage);
        } catch (const std::exception &e) {
            Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Send error: " + std::string(e.what()));
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
            Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Sent request: " + request);
        } catch (const std::exception &e) {
            Logger::getInstance().log("[Floor " + std::to_string(floorNumber) + "] Send error: " + std::string(e.what()));
        }
        state = FloorState::IDLE;
    }
};
 
int main() {
    // For demonstration, simulate floors 1 and 2 in a 22-floor building (Dunton Tower).
    // Create Floor objects for floors 1 and 2.
    Floor floor1(1);
    Floor floor2(2);
 
    // Start processing the same trace file for demonstration.
    // You should update "trace.txt" with appropriate data for a 22-floor building.
    floor1.start("trace.txt");
    floor2.start("trace.txt");
 
    // Wait for the floor simulation threads to finish.
    floor1.join();
    floor2.join();
 
    Logger::getInstance().log("[Floor 1 & 2] Exiting program.");
    return 0;
}
