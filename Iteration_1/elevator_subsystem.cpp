/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "floor_subsystem.hpp"
#include "elevator_subsystem.hpp"
#include "logger.h" 

using namespace std; 

class ElevatorSubsystem {
public: 
    void receiveRequest(FloorRequest& req)
    {
        Logger::logElevatorTask("Received a task: Floor: " + to_string(req.floorNumber) + ", Direction " + req.direction);

        Logger::logElevatorTask("Elevator is moving to Floor " + to_string(req.floorNumber));

        Logger::logElevatorTask("Completed task for Floor " + to_string(req.floorNumber));
    }
};