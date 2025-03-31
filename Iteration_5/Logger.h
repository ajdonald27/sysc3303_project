/**
 * SYSC3303 - Project Iteration 5
 * Authors: David Hos, Aj Donald, Jayven Larsen
 * Date: March 23rd, 2025
 *
 * Logger with millisecond timestamps.
 */

 #ifndef LOGGER_H
 #define LOGGER_H
 
 #include <iostream>
 #include <fstream>
 #include <mutex>
 #include <string>
 #include <chrono>
 #include <ctime>
 #include <sstream>
 #include <iomanip>  // For std::put_time, setw, setfill
 
 class Logger {
 public:
     // Singleton accessor.
     static Logger& getInstance() {
         static Logger instance;
         return instance;
     }
 
     // Log a message with a timestamp in ms.
     void log(const std::string &message) {
         std::lock_guard<std::mutex> lock(logMutex);
         auto now = std::chrono::system_clock::now();
         auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
         std::time_t t = std::chrono::system_clock::to_time_t(now);
         std::tm tm;
     #ifdef _WIN32
         localtime_s(&tm, &t);
     #else
         tm = *std::localtime(&t);
     #endif
         std::ostringstream oss;
         oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
             << '.' << std::setfill('0') << std::setw(3) << ms.count();
         logFile << "[" << oss.str() << "] " << message << std::endl;
         std::cout << "[" << oss.str() << "] " << message << std::endl;
     }
 
 private:
     Logger() {
         logFile.open("unified.log", std::ios::out | std::ios::app);
         if (!logFile.is_open()) {
             std::cerr << "Error opening unified.log for logging." << std::endl;
         }
     }
     ~Logger() {
         if (logFile.is_open())
             logFile.close();
     }
     Logger(const Logger&) = delete;
     Logger& operator=(const Logger&) = delete;
 
     std::ofstream logFile;
     std::mutex logMutex;
 };
 
 #endif // LOGGER_H
 