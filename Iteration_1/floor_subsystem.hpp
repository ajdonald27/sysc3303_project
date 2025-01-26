/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */
#include <string.h>
#include <thread> 
#include <iostream> 
#include <condition_variable> 
#include <mutex> 
#ifndef FLOOR_SUBSYSTEM_HPP
#define FLOOR_SUBSYSTEM_HPP



struct FloorRequest {
    int floorNumber;
    string direction; 
    int elevatorButton;
};

class FloorSubsystem {
public:
    void readRequest_SendScheduler();

    private:
        mutex schedulerMutex; 
        queue<FloorRequest> schedulerQueue; 
        condition_variable schedulerCV; 
};
#endif;