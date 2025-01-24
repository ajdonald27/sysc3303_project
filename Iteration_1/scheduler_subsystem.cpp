/**
 * SYSC3303 - Project Iteration 1: Scheduler Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "floor_subsystem.hpp"
#include "scheduler_subsystem.hpp"
#include "elevator_subsystem.hpp"
class SchedulerSubsystem {
    public: 
        void receiveReqandAssignElevator(FloorRequest& req)
        {
            unique_lock<mutex> lock(schedulerMutex); 


        }


    private:
        mutex schedulerMutex;
        queue<FloorRequest> schedulerQueue;
        condition_variable schedulerCV; 
        ElevatorSubsystem elevatorSub; 
};