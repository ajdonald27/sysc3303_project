/**
 * SYSC3303 - Project Iteration 1: Elevator Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */


// #include <string.h>
// #include <thread> 
// #include <iostream> 
// #include <condition_variable> 
// #include <mutex> 
#include "floor_subsystem.hpp"

#ifndef ELEVATOR_SUBSYSTEM_HPP
#define ELEVATOR_SUBSYSTEM_HPP

class ElevatorSubsystem {
public:
    void receiveRequest(const FloorRequest& req);
    void notifyCompletion(const FloorRequest& req);
private:
    int currentFloor; 
};
#endif // ELEVATOR_SUBSYSTEM_HPP
