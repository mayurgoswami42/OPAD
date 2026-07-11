#include "parser.hpp"

// static members of the Parser
const char Parser::FIELD_SPECIFIER = '$'; // character specifies placeholders
const char Parser::OMITTED_FIELD = '-'; // absent data is marked by - in log line
const char Parser::VALUE_SEPARATOR = ' '; // ' ' is used to seprate different values in log line

Parser::Parser()
{
    avoid_chars.emplace('\"'); // avoid quote
}

// our numbers are clean digits so we dont need heavy case handling, hence efficient than atoi
int Parser::to_int(std::string_view number) const 
{
    int num = 0;
    for (const char &d : number)
    {
        num *= 10;
        num += (d - '0');
    }

    return num;
}

// might needed to avoid some characters in log line, might pe present due to syntax else not useful information
bool Parser::is_avoid_char(char ch) const
{
    if (avoid_chars.find(ch) != avoid_chars.end()) return true; // avoid_characters are stored in avoid_chars hashset when parser generated (written in this case)

    return false;
}

std::pair<std::string_view, std::string_view> Parser::split(std::string_view &s, char ch) const
{   
    int idx = 0;
    while (idx < s.size() && s[idx] != ch) idx++;
    if (idx >= s.size()) return {s, ""}; // ch is not present in s
    
    return {s.substr(0, idx), s.substr(idx+1, s.size()-idx-1)};
}

// value inserted in log object
// id is occurrence index of value in log line
// value is to be inserted
void Parser::insert_log(Log &log, int &id, std::string_view value)
{
    std::string_view log_value = value;
    if (is_avoid_char(value[0])) log_value = log_value.substr(1, log_value.size()-1);
    if (is_avoid_char(value[log_value.size()-1])) log_value = log_value.substr(0, log_value.size()-1);
    
    if (value[0] == OMITTED_FIELD) // value is absent
    {
        ++id;
        return;
    }

    switch (id)
    {
    case LOG_FIELD::DATE:
        log.date = log_value;
        break;
    case LOG_FIELD::TIME:
        log.time = log_value;
        break;
    case LOG_FIELD::LOG_TYPE:
        if (log_value == "DEBUG")
            log.log_type = LogType::DEBUG;
        else if (log_value == "INFO")
            log.log_type = LogType::INFO;
        else if (log_value == "WARN")
            log.log_type = LogType::WARN;
        else if (log_value == "ERROR")
            log.log_type = LogType::ERROR;
        else if (log_value == "FATAL")
            log.log_type = LogType::FATAL;
        else
            log.log_type = LogType::UNKNOWN_LOG;
        break;
    case LOG_FIELD::USER_IP:
        log.user_ip = log_value;
        break;
    case LOG_FIELD::METHOD:
        if (log_value == "GET")
            log.method = Method::GET;
        else if (log_value == "POST")
            log.method = Method::POST;
        else if (log_value == "PUT")
            log.method = Method::PUT;
        else if (log_value == "DELETE")
            log.method = Method::DELETE;
        else
            log.method = Method::UNKNOWN_METHOD;
        break;
    case LOG_FIELD::PATH:
        log.path = log_value;
        break;
    case LOG_FIELD::PROTOCOL:
    {
        auto [protocol, version] = split(log_value, '/');
        ++id;
        if (protocol == "HTTP")
        log.protocol = Protocol::HTTP;
        else if (protocol == "HTTPS")
        log.protocol = Protocol::HTTPS;
        else
        log.protocol = Protocol::UNKNOWN_PROTOCOL;
        log.protocol_version = version;
        break;
    }
    case LOG_FIELD::PROTOCOL_VERSION:
        log.protocol_version = log_value;
        break;
    case LOG_FIELD::STATUS:
        log.status = to_int(log_value);
        break;
    case LOG_FIELD::MESSAGE:
        log.message = log_value;
        break;
    default:
        break;
    }
    ++id;
}

Log Parser::parse(std::string_view log_line)
{
    Log log;
    int id = 0, start = 0, end = 0;

    for (const char &ch : log_line)
    {
        if (ch == VALUE_SEPARATOR)
        {
            insert_log(log, id, log_line.substr(start, end - start));
            start = end + 1;
        }
        end++;
    }

    insert_log(log, id, log_line.substr(start, log_line.size() - start));

    return log;
}

// helper functions (converts enum identifier into string)
// declarations are in log_struct.hpp
std::string log_type_to_string(LogType log_type)
{
    if (log_type == LogType::DEBUG)
    return "DEBUG";
    if (log_type == LogType::INFO)
    return "INFO";
    if (log_type == LogType::WARN)
    return "WARN";
    if (log_type == LogType::ERROR)
    return "ERROR";
    if (log_type == LogType::FATAL)
    return "FATAL";
    if (log_type == LogType::UNKNOWN_LOG)
    return "UNKNOWN_LOG";
    return "";
}

std::string method_to_string(Method method)
{
    if (method == Method::GET)
    return "GET";
    if (method == Method::POST)
    return "POST";
    if (method == Method::PUT)
    return "PUT";
    if (method == Method::DELETE)
    return "DELETE";
    if (method == Method::UNKNOWN_METHOD)
    return "UNKNOWN_METHOD";
    return "";
}

std::string protocol_to_string(Protocol protocol)
{
    if (protocol == Protocol::HTTP)
    return "HTTP";
    if (protocol == Protocol::HTTPS)
    return "HTTPS";
    if (protocol == Protocol::UNKNOWN_PROTOCOL)
    return "UNKNOWN_PROTOCOL";
    
    return "";
}