/**
 * SYSC3303 - Project Iteration 5
 * Authors: David Hos, Aj Donald, Jayven Larsen
 * Date: March 23rd, 2025
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

class Logger {
public:
    // Singleton accessor.
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    // Log a message with a timestamp.
    void log(const std::string &message) {
        std::lock_guard<std::mutex> lock(logMutex);
        auto now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now);
        // Remove trailing newline from ctime.
        std::string timeStr = std::ctime(&now_c);
        if (!timeStr.empty() && timeStr[timeStr.length() - 1] == '\n') {
            timeStr.pop_back();
        }
        logFile << "[" << timeStr << "] " << message << std::endl;
        // Also output to the console.
        std::cout << "[" << timeStr << "] " << message << std::endl;
    }

private:
    Logger() {
        logFile.open("unified.log", std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Error opening unified.log for logging." << std::endl;
        }
    }
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::ofstream logFile;
    std::mutex logMutex;
};

#endif // LOGGER_H
