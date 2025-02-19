/**
 * SYSC3303 - Project Iteration 2 
 * Unit test file 
 * Authors: Aj Donald, XX
 * Date: February 16th, 2025
 */

 #include <iostream>
 #include <cassert>
 #include <thread>
 #include <fstream>
 #include <chrono>
 #include "monitor_implementation.hpp"
 
 using namespace std;
 
 /**
  * test: scheduler populates queue with tasks an input file
  */
 void testSchedulerAddsTasks() 
 {
     

    // create a scheduler instance
     Scheduler scheduler;
 
     // create a temp trace file for testing
     {
         ofstream testFile("test_trace.txt");
         testFile << "12:00:00 3 UP 1\n";
         testFile << "12:00:05 5 DOWN 2\n";
     }
 
     // Launch the scheduler thread to read from the test file
     thread schedulerThread(&Scheduler::processRequests, &scheduler, "test_trace.txt");
 
     // Let the scheduler add tasks (some delay so it can read from file)
     this_thread::sleep_for(chrono::milliseconds(500));
 
     // Since processRequests loops until file ends, we can join
     schedulerThread.join();
 
     // Check if the queue was populated
     // (We can't access the queue directly, but we have hasTasks())
     assert(scheduler.hasTasks() && "Scheduler queue should not be empty after adding tasks.");
     cout << "Test Passed: Scheduler adds tasks correctly." << endl;
 }
 
 /**
  * elevator processes tasks (queue goes empty
  */
 void testElevatorProcessesTasks() 
 {
     // create scheduler and elevator instances
     Scheduler scheduler;
     Elevator elevator(scheduler);
 
     {
         ofstream testFile("test_trace.txt");
         testFile << "12:00:00 3 UP 1\n";
         testFile << "12:00:05 5 DOWN 2\n";
     }
 
     // launch both threads
     thread schedulerThread(&Scheduler::processRequests, &scheduler, "test_trace.txt");
     thread elevatorThread(&Elevator::run, &elevator);
 
     // join them (wait for them to finish)
     schedulerThread.join();
     elevatorThread.join();
 
     // if both are done, queue should be empty
     assert(!scheduler.hasTasks() && "Scheduler queue should be empty after Elevator processes tasks.");
     cout << "Test Passed: Elevator processes tasks correctly." << endl;
 }
 
 int main()
{
     testSchedulerAddsTasks();
     testElevatorProcessesTasks();
 
     cout << "All tests passed successfully." << endl;
     return 0;
 }
 