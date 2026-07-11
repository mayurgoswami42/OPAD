#include <chrono>
#include <format>
#include <string>
#include <sstream>
#include <algorithm>

#include "utils.hpp"

std::string utils::time_str()
{
    auto current_time = std::chrono::system_clock::now();
    std::chrono::zoned_time zt{std::chrono::current_zone(), current_time};
    return std::format("{:%T}", zt);
}

std::string utils::date_str()
{
    return std::format("{:%F}", std::chrono::system_clock::now());
}

utils::time utils::now()
{
    auto current_time = std::chrono::system_clock::now();
    std::chrono::zoned_time zt{std::chrono::current_zone(), current_time};
    return zt;
}

utils::time utils::parse_date_time(const std::string &date, const std::string &time)
{
    std::string fixed_time = time;
    std::replace(fixed_time.begin(), fixed_time.end(), ',', '.');

    // Manual parsing - most reliable
    int year, month, day, hour, minute, second;
    double fraction = 0.0;

    // Parse date
    if (sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        throw std::runtime_error("Failed to parse date: " + date);
    }

    // Parse time
    if (sscanf(fixed_time.c_str(), "%d:%d:%d", &hour, &minute, &second) < 2) {
        throw std::runtime_error("Failed to parse time: " + fixed_time);
    }

    // Parse fractional part
    size_t dot_pos = fixed_time.find('.');
    if (dot_pos != std::string::npos) {
        fraction = std::stod(fixed_time.substr(dot_pos));
    }

    std::tm tm = {};
    tm.tm_year = year - 1900;     // Important!
    tm.tm_mon  = month - 1;       // 0-based
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min  = minute;
    tm.tm_sec  = second;
    tm.tm_isdst = -1;             // Let mktime detect DST

    std::time_t tt = std::mktime(&tm);
    if (tt == -1) {
        throw std::runtime_error("mktime() failed");
    }

    auto tp = std::chrono::system_clock::from_time_t(tt);

    // Add fractional seconds
    auto frac_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::duration<double>(fraction));

    return tp + frac_duration;
}