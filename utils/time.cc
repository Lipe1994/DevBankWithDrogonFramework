#ifndef TIME_UTILS_CC
#define TIME_UTILS_CC
    #include <ctime>
#endif 
#include "time.h"
#include <iostream>
#include <chrono>

std::string TimeUtils::timestampToDateTimeString(std::time_t timestamp) {
    std::tm* timeinfo = std::localtime(&timestamp);

    std::chrono::system_clock::time_point timePoint = std::chrono::system_clock::from_time_t(timestamp);

    auto duration = std::chrono::system_clock::now() - timePoint;

    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count();
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count() % 60;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration).count() % 60;

    char buffer[80];
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);

    return std::string(buffer);
}