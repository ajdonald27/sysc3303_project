/**
 * SYSC3303 - Project Iteration 4
 * Authors: Sami Kasouha
 * Date: March 18th, 2025
 */
#include "elevator.cpp"
#include <cassert>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
#include <vector>

using namespace std;

void sendTestCommand(const string& command, int port) {
    DatagramSocket udpSocket;
    vector<uint8_t> data(command.begin(), command.end());
    DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(port));
    udpSocket.send(packet);
}

void testElevatorMovement() {
    cout << "Running test: Elevator movement...\n";
    thread t([]() {
        Elevator e(1, 0, 9001);
        e.start();
        e.join();
    });

    // Give elevator time to boot up
    this_thread::sleep_for(chrono::milliseconds(500));

    // Send a move command
    sendTestCommand("ASSIGN_ELEVATOR 1 3", 9001);
    this_thread::sleep_for(chrono::seconds(20)); // Wait for elevator to arrive and finish

    sendTestCommand("SHUTDOWN", 9001);
    t.join();
    cout << "Elevator movement test completed.\n";
}

void testStuckElevator() {
    cout << "Running test: Stuck elevator fault...\n";
    thread t([]() {
        Elevator e(1, 0, 9002);
        e.start();
        e.join();
    });

    this_thread::sleep_for(chrono::milliseconds(500));

    // Move to a far floor with low timeout threshold
    sendTestCommand("ASSIGN_ELEVATOR 1 10", 9002);
    this_thread::sleep_for(chrono::seconds(30)); // Wait for stuck detection

    sendTestCommand("SHUTDOWN", 9002);
    t.join();
    cout << "Stuck elevator fault test completed.\n";
}

void testDoorFailure() {
    cout << "Running test: Door failure simulation...\n";
    thread t([]() {
        Elevator e(1, 0, 9003);
        e.start();
        e.join();
    });

    this_thread::sleep_for(chrono::milliseconds(500));
    sendTestCommand("DOOR_RESET 1", 9003);
    this_thread::sleep_for(chrono::seconds(5));

    sendTestCommand("SHUTDOWN", 9003);
    t.join();
    cout << "Door failure reset test completed.\n";
}

void testSensorFailure() {
    cout << "Running test: Sensor failure simulation...\n";
    thread t([]() {
        Elevator e(1, 0, 9004);
        e.start();
        e.join();
    });

    this_thread::sleep_for(chrono::milliseconds(500));
    sendTestCommand("SENSOR_RESET 1", 9004);
    this_thread::sleep_for(chrono::seconds(2));

    sendTestCommand("SHUTDOWN", 9004);
    t.join();
    cout << "Sensor failure simulation test completed.\n";
}

int main() {
    cout << "------ Elevator Unit Tests ------\n";
    testElevatorMovement();
    testStuckElevator();
    testDoorFailure();
    testSensorFailure();
    cout << "------ All Tests Completed ------\n";
    return 0;
}
