SYSC 3303ABC Elevator Control System - Iteration 4
-----------------------------------------------------
Files:
  1. Datagram.h         - UDP helper header (unchanged)
  2. scheduler.cpp      - Scheduler subsystem (listens on UDP port 8000)
  3. elevator.cpp       - Elevator subsystem (listens on UDP port 8001)
  4. floor.cpp          - Floor subsystem (reads requests from trace.txt and sends them via UDP)
  5. trace.txt          - Input file containing floor requests
  6. README.txt         - This file with setup instructions

build instructions:
  - compile each .cpp file separately using:
      g++ -std=c++11 -o scheduler scheduler.cpp
      g++ -std=c++11 -o elevator elevator.cpp
      g++ -std=c++11 -o floor floor.cpp

run instructions:
  - Open three separate terminal windows.
  - In Terminal 1, run the Scheduler:
      ./scheduler
  - In Terminal 2, run the Elevator:
      ./elevator
  - In Terminal 3, run the Floor subsystem:
      ./floor

Note:
  - The current configuration uses localhost (127.0.0.1) for UDP communications.
  - To run on different computers, update the IP addresses in the sendCommand functions accordingly.
  - the trace.txt file is read by the floor subsystem to simulate elevator requests. 
  - Refactored the code from iteration 3 to have a floor subsystem aswell. 


RUNNING USING MULTIPLE HOST: 

In different the different programs, you will make a similar change .
1. In floor.cpp: Change line 74 "            DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000));
" 
Change InetAddress::getLocalHost --> the scheduler IP Address 

2. In elevator.cpp: Change Line 131 DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), htons(8000));


Change InetAddress::getLocalHost --> the IP Address you wish.
