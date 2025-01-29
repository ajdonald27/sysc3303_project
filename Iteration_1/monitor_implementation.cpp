/**
 * SYSC3303 - Project Iteration 1: Main file
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

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

// Task structure to store floor number and direction
struct Task {
    int floorNumber;
    string direction;
};

class Monitor {
public:
    Monitor() : done(false) {}

    void scheduler(const string& filename) {
        ifstream file(filename);
        string line;

        // Read tasks from the file and add them to the queue
        while (getline(file, line)) {
            if (done) break;

            // Parse the trace line
            stringstream ss(line);
            string timestamp;
            int floorNumber;
            string direction;
            int otherValue;

            // Read the timestamp, floor number, direction, and other value
            ss >> timestamp >> floorNumber >> direction >> otherValue;

            // Add a short delay to simulate reading the task
            this_thread::sleep_for(chrono::milliseconds(100));

            {
                lock_guard<mutex> lock(mtx);
                taskQueue.push({floorNumber, direction});
            }

            cout << "Scheduler added task: Floor " << floorNumber << ", Direction: " << direction << endl;
        }

        // Once all tasks are added, notify the elevator to start processing
        {
            lock_guard<mutex> lock(mtx);
            done = true;
        }
        condv.notify_all(); // Notify elevator to start processing
    }

    void elevator() {
        while (true) {
            unique_lock<mutex> lock(mtx);

            // Wait until a task is available or the program is done
            condv.wait(lock, [this] { return !taskQueue.empty() || done; });

            if (done && taskQueue.empty()) {
                break; // Exit if all tasks are processed and done
            }

            // Get the next task from the queue
            Task task = taskQueue.front();
            taskQueue.pop();

            // Simulate elevator processing
            cout << "Elevator is processing task for Floor: " << task.floorNumber
                 << ", Direction: " << task.direction << endl;

            this_thread::sleep_for(chrono::seconds(1)); // Simulate elevator processing time

            cout << "Elevator completed task for Floor: " << task.floorNumber << endl;

            // Signal the scheduler to add more tasks if needed
            condv.notify_all();
        }

        cout << "All tasks completed. Program terminating." << endl;
    }

private:
    mutex mtx;
    condition_variable condv;
    queue<Task> taskQueue;
    bool done;  // Flag to indicate when all tasks are processed
};

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
