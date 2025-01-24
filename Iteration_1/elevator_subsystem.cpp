/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "floor_subsystem.hpp"
#include "elevator_subsystem.hpp"

using namespace std; 

class ElevatorSubsystem {

    void receiveRequest(FloorRequest& req)
    {
        cout << "The elevator has received a request to move to floor " << req.floorNumber 
        << " in direction " << req.direction  << endl; 

        //scheduler.notifyCompletion();
    }

    void notifyCompletion(FloorRequest& req)
    {
        cout << "Elevator has completed the request for floor " << req.floorNumber << endl; 
    }
};