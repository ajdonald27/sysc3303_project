/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */


#ifndef FLOOR_SUBSYSTEM_HPP
#define FLOOR_SUBSYSTEM_HPP

#
#include "scheduler_subsystem.hpp"
#include "floor_request.hpp"

// forward delcaration for compile
//class SchedulerSubsystem;

class FloorSubsystem {
public:

    FloorSubsystem(SchedulerSubsystem& scheduler);
    void readRequest_SendScheduler();

    void receiveRequest(FloorRequest& req);
    private:
        SchedulerSubsystem& schedulerSub;
};
#endif // FLOOR_SUBSYSTEM_HPP