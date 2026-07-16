#include "parser.hpp"

Parser::Parser(const std::string_view &labels_types, const std::string &regex_pattern): regex_obj(regex_pattern)
{
    std::string capture{};
    for (const char &ch : labels_types)
    {
        if (ch == '(')
        {
            capture = "";
            continue;
        }
        if (ch == ')')
        {
            parsed_logs.emplace_back(capture, "");
        }
        capture += ch;
    }
}

std::unordered_map<std::string, std::string> Parser::parse(const std::string &log_line)
{
    int sz = parsed_logs.size();
    std::vector<RE2::Arg> args(sz);
    std::vector<const RE2::Arg*> argv(sz);

    for (int i = 0; i < sz; ++i)
    {
        std::string &ref_second = parsed_logs[i].second;
        ref_second = "";
        args[i] = RE2::Arg(&ref_second);
        argv[i] = &args[i];
    }
    
    RE2::FullMatchN(log_line, regex_obj, argv.data(), sz);

    std::unordered_map<std::string, std::string> map;

    for (const std::pair<std::string, std::string> &pr : parsed_logs)
    {
        map[pr.first] = pr.second;
    }

    return map;
}