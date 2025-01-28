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

    while(getline(file, line))
    {
            // get the data about the floor request from the file 
        istringstream iss(line); 
        string time, floor, direction, button; 
        iss >> time >> floor >> direction >> button; 


        // convert / create a new request based on the read data 
        FloorRequest req = {stoi(floor), direction, stoi(button)};

                
        Logger::logFloorTask("Task added to queue: " + floor + ", Direction " + direction); 


        // add new req to the queue and notify 
        schedulerSub.addToQueue(req); 
    }

    file.close();
}

void receiveRequest(FloorRequest& req)
{
    cout << "FloorSubsystem received a request for Floor: " << req.floorNumber << endl; 
}