#include <iostream>
#include <cassert>
#include "monitor_implementation.hpp"  // Ensure Monitor class is properly included

void testSchedulerAddsTasks() {
    Monitor monitor;
    std::ofstream file("test_trace.txt");
    file << "12:00:00 3 UP 1\n";
    file << "12:00:05 5 DOWN 2\n";
    file.close();

    std::thread schedulerThread(&Monitor::scheduler, &monitor, "test_trace.txt");
    std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Allow time for tasks to be added
    schedulerThread.join();

    assert(!monitor.taskQueue.empty());
    std::cout << "Test Passed: Scheduler adds tasks correctly.\n";
}

void testElevatorProcessesTasks() {
    Monitor monitor;
    std::ofstream file("test_trace.txt");
    file << "12:00:00 3 UP 1\n";
    file << "12:00:05 5 DOWN 2\n";
    file.close();

    std::thread schedulerThread(&Monitor::scheduler, &monitor, "test_trace.txt");
    std::thread elevatorThread(&Monitor::elevator, &monitor);

    schedulerThread.join();
    elevatorThread.join();

    assert(monitor.taskQueue.empty());
    std::cout << "Test Passed: Elevator processes tasks correctly.\n";
}

int main() {
    testSchedulerAddsTasks();
    testElevatorProcessesTasks();

    std::cout << "All tests passed successfully.\n";
    return 0;
}
