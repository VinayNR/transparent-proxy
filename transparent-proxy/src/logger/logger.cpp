#include "logger.h"

#include <cstdio>

/*
* Source Implementation
*/
LogPriority Logger::_priority = DEBUG;
std::mutex Logger::_m;

char* Logger::file_path = nullptr;
std::ofstream Logger::logfile;

void Logger::setPriority(LogPriority priroity) {
    _priority = priroity;
}

void Logger::setFileLogging() {
    setFileLogging("log.txt");
}

void Logger::setFileLogging(const char* file_path) {
    std::cout << "Setting file logging" << std::endl;

    if (!logfile.is_open()) {
        logfile.open(file_path, std::ios::app);
        if (!logfile.is_open()) {
            std::cerr << "Error: Failed to open log file." << std::endl;
        }
    }
}