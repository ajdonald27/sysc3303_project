/**
 * SYSC3303 - Project Iteration 1: Main file
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "floor_subsystem.hpp"
#include "elevator_subsystem.hpp" 
#include "scheduler_subsystem.hpp" 
#include <thread> 
#include <iostream> 

using namespace std; 
int main() 
{ 
    ElevatorSubsystem elevatorSub; 

    SchedulerSubsystem schedulerSub(elevatorSub);

    FloorSubsystem floorSub(schedulerSub);

    thread floorThread([&]
    {
        floorSub.readRequest_SendScheduler();
    });

    thread schedulerThread([&]
    {
        while(true)
        {
            schedulerSub.processTask();
        }
    });

    thread elevatorThread([&]
    {
        while(true)
        {
            // this thread is just processing tasks for the SchedulerSubsystem 
            // Meaning, for this sim each request is processed sequentially through the SchedulerSub's processTask method.
        }
    });
    floorThread.join();
    schedulerThread.join();
    elevatorThread.join();

}