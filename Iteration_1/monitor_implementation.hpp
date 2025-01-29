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

// Task structure to store floor number and direction
struct Task {
    int floorNumber;
    string direction;
};

class Monitor {
public:
    Monitor();
    void scheduler(const string& filename);
    void elevator();
    queue<Task> taskQueue;

private:
    mutex mtx;
    condition_variable condv;
    
    bool done;
};

#endif // MONITOR_IMPLEMENTATION_HPP
