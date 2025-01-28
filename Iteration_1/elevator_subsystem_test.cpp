#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "elevator_subsystem.hpp"
#include "floor_request.hpp"

TEST_CASE("Test ElevatorSubsystem receives and processes requests correctly") {
    ElevatorSubsystem elevatorSub;

    // create a request
    FloorRequest req = {5, "Up", 2};  // Elevator needs to move to floor 5
    // process it 
    elevatorSub.receiveRequest(req);

    // assert that the elevator is at the correct floor after processing 
    CHECK(elevatorSub.getCurrentFloor() == 5); 
}

TEST_CASE("Test ElevatorSubsystem processes multiple requests") {
    ElevatorSubsystem elevatorSub;

    // create multiple floor requests
    FloorRequest req1 = {3, "Up", 1};
    FloorRequest req2 = {6, "Down", 2};

    // process  first request
    elevatorSub.receiveRequest(req1);
    CHECK(elevatorSub.getCurrentFloor() == 3);  
    // process second request
    elevatorSub.receiveRequest(req2);
    CHECK(elevatorSub.getCurrentFloor() == 6);  
}
