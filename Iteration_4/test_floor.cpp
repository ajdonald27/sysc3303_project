/**
 * SYSC3303 - Project Iteration 4
 * Authors: Sami Kasouha
 * Date: March 17th, 2025
 */
#include "floor.cpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;

// Test function using real trace.txt
void testFloorTraceProcessing() {
    cout << "Running test: Floor trace + fault injection using provided trace.txt...\n";

    // Launch the floor system in a thread
    thread floorThread([]() {
        Floor floor(1); // Floor 1 is used in trace.txt
        floor.start();
        floor.join();
    });

    // Let the floor process the trace file
    this_thread::sleep_for(chrono::seconds(10));

    cout << "✔ Floor trace test completed.\n";
    floorThread.join();
}

int main() {
    cout << "=== Floor Subsystem Unit Tests ===\n";
    testFloorTraceProcessing();
    cout << "=== All Floor Tests Completed ===\n";
    return 0;
}
