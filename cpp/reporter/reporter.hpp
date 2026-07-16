#pragma once

#include <string>
#include "detector/detector.hpp"

class Reporter
{
public:
    Reporter() = default;
    std::string get_output(const Detector::Log &log, Anomaly anomaly); // see definition
};