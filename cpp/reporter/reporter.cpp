#include <sstream>

#include "reporter.hpp"
#include "utils/utils.hpp"

std::string stringify(std::string str)
{
    std::string out;
    out += '"';
    for (char &ch : str)
    {
        switch (ch)
        {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += ch;
            break;
        }
    }
    
    out += '"';
    return out;
}

std::string stringify(int num)
{
    return "\"" + std::to_string(num) + "\"";
}

// converts a log and anomaly into json string
std::string Reporter::get_output(const Detector::Log &log, Anomaly anomaly)
{
    std::ostringstream out;
    std::string anomaly_str = anomaly_to_string(anomaly);

    out << "{";
    out << stringify("date") << ":" << stringify(log.at("date")) << ",";
    out << stringify("time") << ":" << stringify(log.at("time")) << ",";
    out << stringify("reported_date") << ":" << stringify(utils::date_str()) << ",";
    out << stringify("reported_time") << ":" << stringify(utils::time_str()) << ",";
    out << stringify("log_type") << ":" << stringify(log.at("log_type")) << ",";
    out << stringify("method") << ":" << stringify(log.at("method")) << ",";
    out << stringify("user_ip") << ":" << stringify(log.at("user_ip")) << ",";
    out << stringify("status") << ":" << stringify(log.at("status")) << ",";
    out << stringify("message") << ":" << stringify(log.at("message")) << ",";
    out << stringify("path") << ":" << stringify(log.at("path")) << ",";
    out << stringify("protocol") << ":" << stringify(log.at("protocol")) << ",";
    out << stringify("protocol_version") << ":" << stringify(log.at("version")) << ",";
    out << stringify("anomaly_type") << ":" << stringify(anomaly_str) << ",";
    out << stringify("anomaly_message") << ":" << stringify("");
    out << "}";

    return out.str();
}