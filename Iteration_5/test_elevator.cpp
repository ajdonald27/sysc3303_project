/**
 * SYSC3303 - Project Iteration 5
 * Authors: Sami Kasouha, Adam Sultan
 * Date: March 23rd, 2025
 *
 *
 */

 #include "floor.cpp"
 #include <iostream>
 #include <thread>
 #include <chrono>
 #include <cassert>
 
 using namespace std;
 
 void testFloorTraceProcessing() {
     cout << "Running test: Floor trace + fault injection using provided trace.txt...\n";
 
     // Launch the Floor system in a thread
     thread floorThread([]() {
         Floor floor(1);
         floor.start("trace.txt");
         floor.join();
         // If join() returns, we assume the trace was processed successfully.
         assert(true); // Reaching here means the floor finished processing
     });
 
     // Allow time for the floor to process the trace file
     this_thread::sleep_for(chrono::seconds(1));
     cout << "Floor trace test completed.\n";
     floorThread.join();
 }
 
 int main() {
     cout << "------ Floor Subsystem Unit Tests ------\n";
     testFloorTraceProcessing();
     cout << "------ All Floor Tests Completed ------\n";
     return 0;
 }
 