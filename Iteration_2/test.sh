#!/usr/bin/env bash

echo Running trace.txt test
g++ -std=c++11 monitor_implementation.cpp -o run
./elevator

echo Running Unit Tests

g++ -std=c++11 -DUNIT_TESTING unit_tests.cpp monitor_implementation.cpp -o unit_tests -pthread
./unit_tests