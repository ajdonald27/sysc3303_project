#!/usr/bin/env bash

echo Running trace.txt test
g++ monitor_implementation.cpp -o elevator
./elevator

echo Running Unit Tests

g++ -DUNIT_TESTING unit_tests.cpp monitor_implementation.cpp -o unit_tests -pthread
./unit_tests