#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "floor_subsystem.hpp"
#include "scheduler_subsystem.hpp"
#include "elevator_subsystem.hpp"
#include "floor_request.hpp"
#include <fstream>

class MockScheduler : public SchedulerSubsystem {
public:
    MockScheduler(ElevatorSubsystem& elevator) : SchedulerSubsystem(elevator) {}
    void addToQueue(FloorRequest &req) {
        SchedulerSubsystem::addToQueue(req);
    }
};

TEST_CASE("Test FloorSubsystem reading requests and adding them to queue") {
    ElevatorSubsystem elevatorSub;
    MockScheduler schedulerSub(elevatorSub);
    FloorSubsystem floorSub(schedulerSub);

    // create a fake trace file for testing
    std::ofstream testFile("trace.txt");
    testFile << "14:05:15.0 2 Up 4\n";
    testFile << "14:06:00.0 6 Down 3\n";
    testFile << "14:10:30.0 11 Up 8\n";
    testFile << "14:15:20.0 17 Down 10\n";
    testFile.close();


    floorSub.readRequest_SendScheduler();

    // check if the tasks are added to the scheduler
    CHECK_FALSE(schedulerSub.isQueueEmpty()); 
}
