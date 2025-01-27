/**
 * SYSC3303 - Project Iteration 1: Main file
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "floor_subsystem.hpp"
#include "elevator_subsystem.hpp" 
#include "scheduler_subsystem.hpp" 
#include <thread> 

using namespace std; 
int main() 
{ 
    SchedulerSubsystem schedulerSub;

    FloorSubsystem floorSub(schedulerSub);
    ElevatorSubsystem elevatorSub; 

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
        //elevatorSub.receiveRequest();
    });
    floorThread.join();
    schedulerThread.join();
    

}