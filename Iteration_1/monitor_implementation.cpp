#include "monitor_implementation.hpp"

// Constructor
Monitor::Monitor() : done(false) {}

// Scheduler function
void Monitor::scheduler(const string& filename) {
    ifstream file(filename);
    string line;

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

// Elevator function
void Monitor::elevator() {
    while (true) {
        unique_lock<mutex> lock(mtx);
        condv.wait(lock, [this] { return !taskQueue.empty() || done; });

        if (done && taskQueue.empty()) {
            break;
        }

        Task task = taskQueue.front();
        taskQueue.pop();

        cout << "Elevator is processing task for Floor: " << task.floorNumber
             << ", Direction: " << task.direction << endl;

        this_thread::sleep_for(chrono::seconds(1));

        cout << "Elevator completed task for Floor: " << task.floorNumber << endl;

        condv.notify_all();
    }

    cout << "All tasks completed. Program terminating." << endl;
}

// Main function
#ifndef UNIT_TESTING
int main() {
    Monitor monitor;

    // Create the scheduler and elevator threads
    thread schedulerThread(&Monitor::scheduler, &monitor, "trace.txt"); // Assuming "trace.txt" is your input file
    thread elevatorThread(&Monitor::elevator, &monitor);

    // Wait for both threads to finish
    schedulerThread.join();
    elevatorThread.join();

    return 0;
}
#endif
