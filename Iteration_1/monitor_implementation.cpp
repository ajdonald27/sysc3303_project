/**
* SYSC3303 - Project Iteration 1: Elevator 
* Authors: Aj Donald, Adam Sultan, XX
* Date : January 28th, 2025
*/
#include "monitor_implementation.hpp"

// class constructor 
Monitor::Monitor() : done(false) {}

// Scheduler function
void Monitor::scheduler(const string& filename) {
    ifstream file(filename);
    string line;

    // parse the trace file (.txt)
    while (getline(file, line)) {
        if (done) break;

        stringstream ss(line);
        string timestamp;
        int floorNumber;
        string direction;
        int otherValue;

        ss >> timestamp >> floorNumber >> direction >> otherValue;

        this_thread::sleep_for(chrono::milliseconds(100));

        {
            lock_guard<mutex> lock(mtx);
            taskQueue.push({floorNumber, direction});
        }

        cout << "Scheduler added task: Floor " << floorNumber << ", Direction: " << direction << endl;
    }

    {
        lock_guard<mutex> lock(mtx);
        done = true;
    }
    condv.notify_all();
}

// elevator function
void Monitor::elevator() {
    // infinite loop for processing requests
    while (true) {
        // 
        unique_lock<mutex> lock(mtx);
        condv.wait(lock, [this] { return !taskQueue.empty() || done; });

        if (done && taskQueue.empty()) {
            break;
        }

    // bring to the front of the queue, then remove it once processed
        Task task = taskQueue.front();
        taskQueue.pop();

        cout << "Elevator is processing task for Floor: " << task.floorNumber
             << ", Direction: " << task.direction << endl;

        // add 1 second delay
        this_thread::sleep_for(chrono::seconds(1));

        
        cout << "Elevator completed task for Floor: " << task.floorNumber << endl;

        condv.notify_all();
    }

    cout << "All tasks completed. Program terminating" << endl;
}

// main function 
// guard for unit tests.
#ifndef UNIT_TESTING
int main() {
    Monitor monitor;

    // creating the scheduler / input threads
    thread schedulerThread(&Monitor::scheduler, &monitor, "trace.txt");
    thread elevatorThread(&Monitor::elevator, &monitor);

    // wait for threads to finish
    schedulerThread.join();
    elevatorThread.join();

    return 0;
}
#endif
