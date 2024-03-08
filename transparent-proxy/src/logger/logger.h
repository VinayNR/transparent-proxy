#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <ctime>

enum LogPriority {
    DEBUG, WARN, INFO, ERROR
};

class Logger {
    private:
        static LogPriority _priority;
        static std::mutex _m;

        static char *file_path;
        static std::ofstream logfile;

        // Helper function to print each argument
        template<typename T, typename... Rest>
        static void printArgs(std::ostringstream& oss, const T& firstArg, const Rest&... rest) {
            oss << firstArg;
            printArgs(oss, rest...); // Recursively call printArgs for remaining arguments
        }

        // Base case for the recursive template function
        static void printArgs(std::ostringstream& oss) {}

        template <typename... Args>
        static void writeLog(const char * message_str, LogPriority message_priority, Args... args) {
            if (_priority <= message_priority) {
                std::scoped_lock lock(_m);
                std::ostringstream oss;

                // log timestamp
                // Get current time
                std::time_t currentTime = std::time(nullptr);

                // Convert current time to local time
                std::tm* localTime = std::localtime(&currentTime);

                // Print the current time
                oss << (localTime->tm_hour < 10 ? "0" : "") << localTime->tm_hour << ":"
                        << (localTime->tm_min < 10 ? "0" : "") << localTime->tm_min << ":"
                        << (localTime->tm_sec < 10 ? "0" : "") << localTime->tm_sec
                        << "  ";

                // log thread ID
                oss << "[" << std::this_thread::get_id() << "]  ";

                // add the message string
                oss << message_str;

                // append all arguments to the output stream
                printArgs(oss, args...);

                // add a new line
                oss << "\n";

                // print the message to console output
                std::cout << oss.str();

                // Write the message to the log file
                if (logfile.is_open()) {
                    logfile << oss.str();
                    logfile.flush();
                }
            }
        }

    public:

        static void setPriority(LogPriority);

        static void setFileLogging();
        static void setFileLogging(const char *);

        template <typename... Args>
        static void debug(const Args&... a) {
            writeLog("DEBUG\t", DEBUG, a...);
        }

        template <typename... Args>
        static void warn(const Args&... a) {
            writeLog("WARN\t", WARN, a...);
        }

        template <typename... Args>
        static void info(const Args&... a) {
            writeLog("INFO\t", INFO, a...);
        }

        template <typename... Args>
        static void error(const Args&... a) {
            writeLog("ERROR\t", ERROR, a...);
        }
};