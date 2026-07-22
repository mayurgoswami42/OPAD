#include <sstream>

#include "reporter.hpp"
#include "utils/utils.hpp"

// converts a log and anomaly into json string
std::string Reporter::get_output(const Detector::Log &log, Anomaly anomaly)
{
    std::ostringstream out;
    std::string anomaly_str = anomaly_to_string(anomaly);

    out << "{";
    out << utils::stringify("date") << ":" << utils::stringify(log.at("date")) << ",";
    out << utils::stringify("time") << ":" << utils::stringify(log.at("time")) << ",";
    out << utils::stringify("reported_date") << ":" << utils::stringify(utils::date_str()) << ",";
    out << utils::stringify("reported_time") << ":" << utils::stringify(utils::time_str()) << ",";
    out << utils::stringify("log_type") << ":" << utils::stringify(log.at("log_type")) << ",";
    out << utils::stringify("method") << ":" << utils::stringify(log.at("method")) << ",";
    out << utils::stringify("user_ip") << ":" << utils::stringify(log.at("user_ip")) << ",";
    out << utils::stringify("status") << ":" << utils::stringify(log.at("status")) << ",";
    out << utils::stringify("message") << ":" << utils::stringify(log.at("message")) << ",";
    out << utils::stringify("path") << ":" << utils::stringify(log.at("path")) << ",";
    out << utils::stringify("protocol") << ":" << utils::stringify(log.at("protocol")) << ",";
    out << utils::stringify("protocol_version") << ":" << utils::stringify(log.at("version")) << ",";
    out << utils::stringify("anomaly_type") << ":" << utils::stringify(anomaly_str) << ",";
    out << utils::stringify("anomaly_message") << ":" << utils::stringify("");
    out << "}";

    return out.str();
}