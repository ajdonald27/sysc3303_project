/**
 * SYSC3303 - Project Iteration 2
 * Authors: Jayven Larsen, Aj Donald
 * Date: February 16th, 2025
 */

#include "monitor_implementation.hpp"
Scheduler::Scheduler() : currentSchedulerState(SchedulerState::IDLE), done(false) {}

void Scheduler::processRequests(const string& filename) 
{

    // get the file to parse the data 
    ifstream file(filename);
    string line;

    // loop over the file 
    while (getline(file, line)) {
        if (done) break;

        // variable declarations
        stringstream ss(line);
        string timestamp;
        int floorNumber;
        string direction;
        int priority;

        ss >> timestamp >> floorNumber >> direction >> priority;

        this_thread::sleep_for(chrono::milliseconds(100));

        {
            lock_guard<mutex> lock(mtx);
            taskQueue.push({floorNumber, direction, priority});
            currentSchedulerState = SchedulerState::ASSIGNING_TASK;
        }

        condv.notify_one();
        cout << "[Scheduler] New Task Added: " << timestamp << " Floor " << floorNumber << " (" << direction << ") Priority: " << priority << endl;
    }

    {
        lock_guard<mutex> lock(mtx);
        done = true;
    }
    condv.notify_all();
}

// helper function to get the next task
Task Scheduler::getNextTask() {
    lock_guard<mutex> lock(mtx);
    if (taskQueue.empty()) return {-1, "", -1};

    currentSchedulerState = SchedulerState::WAITING_FOR_RESPONSE;
    Task task = taskQueue.front();
    taskQueue.pop();
    return task;
}

void Scheduler::notifyElevator() {
    lock_guard<mutex> lock(mtx);
    cout << "[Scheduler] Elevator has arrived at a floor." << endl;

    currentSchedulerState = taskQueue.empty() ? SchedulerState::IDLE : SchedulerState::ASSIGNING_TASK;
    condv.notify_one();
}

bool Scheduler::hasTasks() {
    lock_guard<mutex> lock(mtx);
    return !taskQueue.empty();
}

// elevator subsystem
Elevator::Elevator(Scheduler& scheduler) : scheduler(scheduler), currentElevatorState(ElevatorState::IDLE), currentFloor(0) {}

void Elevator::run() {
    // indefinite loop 
    while (true) {
        unique_lock<mutex> lock(mtx);
        scheduler.condv.wait(lock, [this] { return scheduler.hasTasks() || scheduler.done; });

        if (scheduler.done && !scheduler.hasTasks()) break;

        Task task = scheduler.getNextTask();
        if (task.floorNumber == -1) continue;

        if (task.floorNumber > currentFloor) {
            currentElevatorState = ElevatorState::MOVING_UP;
            cout << "[Elevator] Moving UP to Floor " << task.floorNumber << endl;
        } else if (task.floorNumber < currentFloor) {
            currentElevatorState = ElevatorState::MOVING_DOWN;
            cout << "[Elevator] Moving DOWN to Floor " << task.floorNumber << endl;
        } else {
            currentElevatorState = ElevatorState::ARRIVED;
        }

        this_thread::sleep_for(chrono::seconds(1));

        currentFloor = task.floorNumber;
        currentElevatorState = ElevatorState::ARRIVED;
        cout << "[Elevator] Arrived at Floor " << currentFloor << " Priority: " << task.priority << endl;

        scheduler.notifyElevator();
        currentElevatorState = ElevatorState::IDLE;
    }

    cout << "[Elevator] All tasks completed. Program terminating." << endl;
}

#ifndef UNIT_TESTING
int main() {
    // create instances of both a scheduler and an elevator
    Scheduler scheduler;
    Elevator elevator(scheduler);

    // create the threads + join them 
    thread schedulerThread(&Scheduler::processRequests, &scheduler, "trace.txt");
    thread elevatorThread(&Elevator::run, &elevator);
    schedulerThread.join();
    elevatorThread.join();

    return 0;
}
#endif
