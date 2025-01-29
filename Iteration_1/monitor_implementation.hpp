/**
 * SYSC3303 - Project Iteration 1 : Header File
 * Authors: Aj Donald, Adam Sultan
 * Date: January 28th, 2025
 */

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

// structure to process tasks from a trace file
struct Task {
    int floorNumber;
    string direction;
};

// class definition for the monitor
class Monitor {
public:
    Monitor();
    void scheduler(const string& filename);
    void elevator();
    queue<Task> taskQueue;

// class attributes 
private:
    mutex mtx;
    condition_variable condv;
    
    bool done;
};

#endif // MONITOR_IMPLEMENTATION_HPP
