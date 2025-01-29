/**
 * SYSC3303 - Project Iteration 1 
 * Authors: Aj Donald, Adam Sultan, XX 
 * Date: January 28th, 2025
 */

#include <iostream>
// include for the unit test
#include <cassert>
#include "monitor_implementation.hpp"

// test for tasks being added to the queue
void testSchedulerAddsTasks() {
    // need to populate a test_trace file to call the monitor functions
    Monitor monitor;
    std::ofstream file("test_trace.txt");
    file << "12:00:00 3 UP 1\n";
    file << "12:00:05 5 DOWN 2\n";
    file.close();

    // pass the new trace file 
    std::thread schedulerThread(&Monitor::scheduler, &monitor, "test_trace.txt");
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
    schedulerThread.join();

    // make sure the queue got populated correctly
    assert(!monitor.taskQueue.empty());
    std::cout << "Test Passed: Scheduler adds tasks correctly.\n";
}

// testing a round of tasks to process 
void testElevatorProcessesTasks() {

    // same as previous test, populated test trace file
    Monitor monitor;
    std::ofstream file("test_trace.txt");
    file << "12:00:00 3 UP 1\n";
    file << "12:00:05 5 DOWN 2\n";
    file.close();

    std::thread schedulerThread(&Monitor::scheduler, &monitor, "test_trace.txt");
    std::thread elevatorThread(&Monitor::elevator, &monitor);

    schedulerThread.join();
    elevatorThread.join();

    // make sure all the tasks were processed (as in queue is empty)
    assert(monitor.taskQueue.empty());
    std::cout << "Test Passed: Elevator processes tasks correctly.\n";
}

int main() {
    testSchedulerAddsTasks();
    testElevatorProcessesTasks();

    std::cout << "All tests passed successfully.\n";
    return 0;
}
