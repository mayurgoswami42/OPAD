#include <iostream>
#include <thread>

#include "io_manager/reader.hpp"
#include "parser/parser.hpp"
#include "detector/detector.hpp"
#include "reporter/reporter.hpp"

#include "socket_server/socket_server.hpp"

#include "utils/debug.hpp"

[[maybe_unused]] void print(Detector::Log log) // debuging purpose only
{
    std::cout << "date:" << log.at("date") << std::endl;
    std::cout << "time:" << log.at("time") << std::endl;
    std::cout << "log_type:" << log.at("log_type") << std::endl;
    std::cout << "user_ip:" << log.at("user_ip") << std::endl;
    std::cout << "status:" << log.at("status") << std::endl;
    std::cout << "message:" << log.at("message") << std::endl;
    std::cout << "method:" << log.at("method") << std::endl;
    std::cout << "path:" << log.at("path") << std::endl;
    std::cout << "protocol:" << log.at("protocol") << std::endl;
    std::cout << "protocol_version:" << log.at("protocol_version") << std::endl;
    std::cout << "\n\n" << std::endl;
}

int detection_loop(std::string log_path, SocketServer *sock_server)
{
    Reader reader;
    DEBUG_DECLARE(reader.reset_offset());

    Parser parser("(date)(time)(log_type)(user_ip)(method)(path)(protocol)(version)(status)(message)", R"re((\d{4}-\d{2}-\d{2})\s+((?:\d{2}:){2}\d{2},\d{3})\s+(INFO|ERROR|WARN|FATAL)\s+((?:\d{1,3}\.){3}\d{1,3})\s+"(GET|POST|PUT|DELETE|PATCH)\s+(\S+)\s+((?:HTTP|HTTPS)/(\d(?:\.\d)?))"\s+(\d{3})\s+(\S+))re");
    Detector detector(50000, 5000, 30);
    Reporter reporter;

    while (true)
    {
        std::vector<std::string> log_lines = reader.read_logs(log_path);
        if (!reader.status)
        {
            DEBUG_LOG("MAIN::ERROR:: Reader returned with bad status! (line " << __LINE__ << ")");
            return 1;
        }

        for (std::string &s : log_lines)
        {
            std::unordered_map<std::string, std::string> parsed_log = parser.parse(s);
            (detector.*detector.insert)(parsed_log); // insert is a pointer function switches between do_insert and process, to efficiently process logs
            int sz = detector.sus_log_counts(); // size of suspicious_logs
            if (sz > 0)
            {
                for (const std::pair<Detector::Log, Anomaly> sus_log : detector.get_sus_logs())
                {
                    std::string output = reporter.get_output(sus_log.first, sus_log.second);
                    sock_server->add_task(output);
                    DEBUG_LOG("suspicious line: " << s);
                }
                detector.reset();
            }
        }
    }

    return 0;
}

void serve_forever(SocketServer *sock_server)
{
    while (true)
    {
        sock_server->serve();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "cpp:exit:1";
        return 0;
    }

    SocketServer sock_server(5555);

    std::thread detection(detection_loop, argv[1], &sock_server);
    std::thread server_thread(serve_forever, &sock_server);

    DEBUG_LOG("All logs are parsed and detector scanned!");
    server_thread.join();

    return 0;
}