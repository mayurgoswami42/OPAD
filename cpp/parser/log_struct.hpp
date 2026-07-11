#pragma once

#include <string>
#include <cstdint>

// the order of identifiers should match the order in log line
enum LOG_FIELD
{
    DATE,
    TIME,
    LOG_TYPE,
    USER_IP,
    METHOD,
    PATH,
    PROTOCOL,
    PROTOCOL_VERSION,
    STATUS,
    MESSAGE
};

enum LogType
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    UNKNOWN_LOG
};

enum Method
{
    GET,
    POST,
    PUT,
    DELETE,
    UNKNOWN_METHOD
};

enum Protocol
{
    HTTP,
    HTTPS,
    UNKNOWN_PROTOCOL
};

struct Log // python http.server specific log struct
{
    std::string date{};
    std::string time{};
    LogType log_type{};
    std::string user_ip{};
    uint32_t status{};
    std::string message{};
    Method method{};
    std::string path{};
    Protocol protocol{};
    std::string protocol_version{};
};

// definations are in parser.cpp
// converts every enum identifier into string
std::string log_type_to_string(LogType log_type);
std::string method_to_string(Method method);
std::string protocol_to_string(Protocol protocol);