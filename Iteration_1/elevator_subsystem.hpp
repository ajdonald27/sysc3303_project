/**
 * SYSC3303 - Project Iteration 1: Elevator Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */


#ifndef ELEVATOR_SUBSYSTEM_HPP
#define ELEVATOR_SUBSYSTEM_HPP

#include "floor_request.hpp"

class ElevatorSubsystem {
public:

    ElevatorSubsystem(); 

    void receiveRequest(FloorRequest& req);

    void simulateMovement(FloorRequest& req);

    int getCurrentFloor();

private:
    int currentFloor;
};
#endif
