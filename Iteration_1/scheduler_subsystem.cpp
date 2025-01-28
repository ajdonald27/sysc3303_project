/**
 * SYSC3303 - Project Iteration 1: Scheduler Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

#include "logger.hpp"
#include "scheduler_subsystem.hpp"

using namespace std;


SchedulerSubsystem::SchedulerSubsystem(ElevatorSubsystem& elevator)  : elevatorSubsystem(elevator), ready(false), completed(false) {}


void SchedulerSubsystem::addToQueue(FloorRequest& req)
    {

    // scheduler waits on the mutex, then checks if the queue is empty or not 
    unique_lock<mutex> lock(schedulerMutex); 
            

    schedulerQueue.push(req); 
    // printout for message received and sending request to the right user.
    Logger::logSchedulerTask("Task added to queue: Floor " + to_string(req.floorNumber) + ", Direction " + req.direction); 

    ready = true; 
    schedulerCV.notify_one(); 
    }

void SchedulerSubsystem::processTask()
{
    unique_lock<mutex> lock(schedulerMutex);

    while (!schedulerQueue.empty()) 
    { 
        schedulerCV.wait(lock, [this] { return !schedulerQueue.empty() || completed; });

        if (schedulerQueue.empty()) {
            completed = true; 
            break;
        }

        FloorRequest req = schedulerQueue.front();
        schedulerQueue.pop();
        elevatorSubsystem.receiveRequest(req);
    }
}
bool SchedulerSubsystem::isQueueEmpty()
{
    return schedulerQueue.empty(); 
}