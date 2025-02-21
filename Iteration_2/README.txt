SYSC3303 - Project Iteration 2
Group 7
Date: February 16th , 2025
Members: Aj Donald, Adam Sultan, Jayven Larsen, David Hos, Sami Kasouha. 

The file structure for this iteration of the project is relatively simple. 
Similarly to assignment 1, a monitor was used in the implementation to have the threads interaction with eachother. 

Class Definitions, includes and struct declarations can be found in :monitor_implementation.hpp

Implementation is confined to the monitor_implementation.cpp file, this includes a main function as well. Note, this implementatiom comes with a trace file "trace.txt"
The data in this file is read and used for processing throughout program execution.

In terms of UNIT Testing, it's based off of Google Test / cassert library. 
We've included a test_trace.txt file. This file gets populated when you try to run UNIT tests on this code.

All of the tests are included in unit_test.cpp

Next, we've included a bash script which contains compilation commands for both implementation and test files. Otherwise, there's instructions to compile these programs manually as well. 










