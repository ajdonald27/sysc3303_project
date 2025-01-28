/**
 * SYSC3303 - Project Iteration 1: Scheduler Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#ifndef SCHEDULER_SUBSYSTEM_HPP
#define SCHEDULER_SUBSYSTEM_HPP

#include "floor_request.hpp"
#include "elevator_subsystem.hpp"
#include <queue> 
#include <condition_variable> 
#include <mutex> 

using namespace std; 

class SchedulerSubsystem { 
    public: 
        SchedulerSubsystem(ElevatorSubsystem& elevator);

        void addToQueue(FloorRequest &req);
        void processTask(); 
        bool isQueueEmpty(); 

        bool completed; 
    private:
        mutex schedulerMutex;
        queue<FloorRequest> schedulerQueue;
        condition_variable schedulerCV; 
        ElevatorSubsystem& elevatorSubsystem; 
        bool ready; 
        
};
#endif

