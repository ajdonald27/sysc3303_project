#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "scheduler_subsystem.hpp"
#include "elevator_subsystem.hpp"


class MockElevator : public ElevatorSubsystem {
public:
    MockElevator() : ElevatorSubsystem() {}
    void receiveRequest(FloorRequest& req) {
        // sim the elevator receiving the request
        std::cout << "Elevator processing request for Floor: " << req.floorNumber << std::endl;
    }
};

TEST_CASE("Test SchedulerSubsystem processes tasks and forwards them to ElevatorSubsystem") {
    MockElevator elevatorSub;
    SchedulerSubsystem schedulerSub(elevatorSub);

    // add a request 
    FloorRequest req = {2, "Up", 4};
    schedulerSub.addToQueue(req);

    // process req 
    schedulerSub.processTask();

    // make sure the queue is empty
    CHECK(schedulerSub.isQueueEmpty());
}
