/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */
#include <string>
#include <thread> 
#include <iostream> 
#include <condition_variable> 
#include <mutex> 
#include <fstream> 
#include <sstream> 
#include <queue> 
#include "logger.h" 

#ifndef FLOOR_SUBSYSTEM_HPP
#define FLOOR_SUBSYSTEM_HPP



struct FloorRequest {
    int floorNumber;
    string direction; 
    int elevatorButton;
};

class FloorSubsystem {
public:

    FloorSubsystem(SchedulerSubsystem& scheduler) : schedulerSub(scheduler) {}
    void readRequest_SendScheduler();

    private:
        SchedulerSubsystem& schedulerSub;
};
#endif;