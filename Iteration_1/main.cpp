/**
 * SYSC3303 - Project Iteration 1: Main file
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "floor_subsystem.hpp"
#include "elevator_subsystem.hpp" 
#include "scheduler_subsystem.hpp" 

using namespace std; 
int main() 
{ 
    FloorSubsystem floorSub;
    ElevatorSubsystem elevatorSub; 
    SchedulerSubsystem schedulerSub;

    thread floorThread([&]
    {
        floorSub.readRequest_SendScheduler();
    });

    thread schedulerThread([&]
    {
        schedulerSub.receiveReqandAssignElevator();
    });

    floorThread.join();
    schedulerThread.join();
    

}