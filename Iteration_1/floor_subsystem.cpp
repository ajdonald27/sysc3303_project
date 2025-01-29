/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */


#include "floor_subsystem.hpp"
#include "logger.hpp"
#include <fstream> 
#include <sstream> 
#include <string> 
#include <iostream> 



using namespace std; 

FloorSubsystem::FloorSubsystem(SchedulerSubsystem& scheduler) : schedulerSub(scheduler) {}

void FloorSubsystem::readRequest_SendScheduler()
{
    ifstream file("trace.txt");
    string line;
    bool read = false;
    while (getline(file, line)) {
        
        // parsing the data from the trace file 
        istringstream iss(line);
        string time, floor, direction, button;
        iss >> time >> floor >> direction >> button;

        // convert and create a new request
        FloorRequest req = {stoi(floor), direction, stoi(button)};
                
        Logger::logFloorTask("Task added to queue: " + floor + ", Direction " + direction);
        schedulerSub.addToQueue(req);
    }

    file.close();
    read = true;
    // After adding all tasks notify the scheduler to process them
    if (read && schedulerSub.isQueueEmpty()) {
        schedulerSub.completed = true; 
    }
}


void receiveRequest(FloorRequest& req)
{
    cout << "FloorSubsystem received a request for Floor: " << req.floorNumber << endl; 
}