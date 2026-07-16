#pragma once

#include <string>
#include <chrono>
#include <utility>

namespace utils
{
    typedef std::chrono::system_clock::time_point time;
    typedef std::chrono::duration<double> duration;
    
    std::string time_str();
    std::string date_str();

    utils::time parse_date_time(const std::string &date, const std::string &time);

    utils::time now();
}