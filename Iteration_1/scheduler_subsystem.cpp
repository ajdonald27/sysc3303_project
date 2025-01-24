/**
 * SYSC3303 - Project Iteration 1: Scheduler Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "floor_subsystem.hpp"
#include "elevator_subsystem.hpp"
#include "scheduler_subsystem.hpp"

using namespace std;

class SchedulerSubsystem {
    public: 
        void receiveReqandAssignElevator(FloorRequest& req)
        {
            // scheduler waits on the mutex, then checks if the queue is empty or not 
            unique_lock<mutex> lock(schedulerMutex); 
            schedulerCV.wait(lock, [this]{ return !schedulerQueue.empty();});

            FloorRequest req = schedulerQueue.front(); 
            schedulerQueue.pop();

            // printout for message received and sending request to the right user.
            cout << "Scheduler assigning request to elevator: " << " Floor " << req.floorNumber << ", Direction" << req.direction << endl; 
            elevatorSubsystem.receiveRequest(req); 
        }


    private:
        mutex schedulerMutex;
        queue<FloorRequest> schedulerQueue;
        condition_variable schedulerCV; 
        ElevatorSubsystem elevatorSubsystem; 
};