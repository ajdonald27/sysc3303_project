/**
 * SYSC3303 - Project Iteration 1: Floor Subsystem
 * Authors: Aj Donald, XX, XX 
 * Date: January 23rd, 2025
 */

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

