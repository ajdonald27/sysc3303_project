/**
 * SYSC3303 - Project Iteration 1: Scheduler Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */


#include <string.h>
#include <thread> 
#include <iostream> 
#include <condition_variable> 
#include <mutex> 
#include "floor_subsystem.hpp"
using namespace std; 

#ifndef SCHEDULER_SUBSYSTEM_HPP
#define SCHEDULER_SUBSYSTEM_HPP


class SchedulerSubsystem { 
    public: 
        void receiveReqandAssignElevator();



    private:
        mutex schedulerMutex; 
        queue<FloorRequest> schedulerQueue; 
        condition_variable schedulerCV; 
};
#endif; 

