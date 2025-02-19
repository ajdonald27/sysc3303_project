#ifndef MONITOR_IMPLEMENTATION_HPP
#define MONITOR_IMPLEMENTATION_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <vector>
#include <string>

using namespace std;

// Possible states for the scheduler
enum class SchedulerState { IDLE, ASSIGNING_TASK, WAITING_FOR_RESPONSE };

// Possible states for the elevator
enum class ElevatorState { IDLE, MOVING_UP, MOVING_DOWN, ARRIVED };

// Task structure representing an elevator request
struct Task {
    int floorNumber;
    string direction;
    int priority;

    Task(int floorNumber, const string& direction, int priority)
        : floorNumber(floorNumber), direction(direction), priority(priority) {}
};



// Scheduler class to manage requests
class Scheduler {
public:
    
    Scheduler();
    void processRequests(const string& filename);
    void notifyElevator();
    bool hasTasks();
    Task getNextTask();
    SchedulerState currentSchedulerState;

private:
    queue<Task> taskQueue;
    mutex mtx;
    condition_variable condv;
    SchedulerState state;
    bool done;

    friend class Elevator; // Allow Elevator to access private members
};

// Elevator class to handle movement
class Elevator {
public:
    Elevator(Scheduler& scheduler);
    void run();

private:
    Scheduler& scheduler;
    mutex mtx;
    condition_variable condv;
    ElevatorState currentElevatorState;
    int currentFloor;
};

#endif 
