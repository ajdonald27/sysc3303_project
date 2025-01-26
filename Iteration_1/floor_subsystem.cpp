/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */


#include "floor_subsystem.hpp"
#include "elevator_subsystem.hpp"
#include "scheduler_subsystem.hpp"


using namespace std; 

class FloorSubsystem { 
    public: 
        void readRequest_SendScheduler()
        {
            FloorRequest req = {2, "up", 3}; 
            {
                lock_guard<mutex> lock(schedulerMutex);
                schedulerQueue.push(req);
            }
            // notify the scheduler about the new request in the queue
            schedulerCV.notify_one();
        }


    private:
        mutex schedulerMutex; 
        queue<FloorRequest> schedulerQueue; 
        condition_variable schedulerCV; 
};
