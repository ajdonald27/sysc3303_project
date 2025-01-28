/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "elevator_subsystem.hpp"
#include "logger.hpp"
#include <iostream>
#include <thread> 
#include <chrono> 


using namespace std; 

ElevatorSubsystem::ElevatorSubsystem() {}

void ElevatorSubsystem::receiveRequest(FloorRequest& req)
{
    Logger::logElevatorTask("Received a task: Floor: " + to_string(req.floorNumber) + ", Direction " + req.direction);

    simulateMovement(req); 

    Logger::logElevatorTask("Completed task for Floor " + to_string(req.floorNumber));
}

void ElevatorSubsystem::simulateMovement(FloorRequest& req)
{
    cout << "Elevator is moving to Floor: " << req.floorNumber << endl; 
    this_thread::sleep_for(chrono::seconds(1)); 
}
