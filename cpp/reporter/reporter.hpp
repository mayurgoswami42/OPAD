#pragma once

#include <string>
#include "detector/anomaly_struct.hpp"
#include "parser/log_struct.hpp"

class Reporter
{
public:
    Reporter() = default;
    std::string get_output(Log log, Anomaly anomaly); // see defination
};