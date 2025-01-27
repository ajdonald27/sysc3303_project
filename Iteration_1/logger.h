
#ifndef LOGGER_H 
#define LOGGER_H 

#include <iostream> 
#include <string> 

using namespace std; 
class Logger { 
    public: 
    
    static void logFloorTask(string task)
    {
        cout << "Floor Subsystem: " << task << endl; 
    }

    static void logSchedulerTask(string task)
    {
        cout << "Scheduler::run: " << task << endl; 

    }
    static void logElevatorTask(string task)
    {
        cout << "Elevator: " << task << endl;
    }
};
#endif 