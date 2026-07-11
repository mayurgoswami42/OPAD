#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <utility>
#include <unordered_set>

#include "log_struct.hpp"

class Parser
{
public:
    Parser();
    Log parse(std::string_view);

private:
    static const char FIELD_SPECIFIER; // character specifies placeholders
    static const char OMITTED_FIELD; // absent data is marked by - in log line
    static const char VALUE_SEPARATOR; // ' ' is used to seprate different values in log line

    std::unordered_set<char> avoid_chars;

    int to_int(std::string_view) const; // efficient and simpler input atoi
    bool is_avoid_char(char) const;
    std::pair<std::string_view, std::string_view> split(std::string_view &, char) const;
    void insert_log(Log &, int&, std::string_view);
};