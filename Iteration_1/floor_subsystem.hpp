/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */
#ifndef FLOOR_SUBSYSTEM_HPP
#define FLOOR_SUBSYSTEM_HPP
#include <string.h>
#include <thread> 
#include <iostream> 
#include <condition_variable> 
#include <mutex> 


struct FloorRequest {
    int floorNumber;
    string direction; 
    int elevatorButton;
};

#endif;