#include <iostream>
#include <thread>

#include "io_manager/reader.hpp"
#include "parser/parser.hpp"
#include "detector/detector.hpp"
#include "reporter/reporter.hpp"

#include "socket_server/socket_server.hpp"

#include "utils/debug.hpp"

[[maybe_unused]] void print(Log log) // debuging purpose only
{
    std::cout << "date:" << log.date << std::endl;
    std::cout << "time:" << log.time << std::endl;
    std::cout << "log_type:" << log.log_type << std::endl;
    std::cout << "user_ip:" << log.user_ip << std::endl;
    std::cout << "status:" << log.status << std::endl;
    std::cout << "message:" << log.message << std::endl;
    std::cout << "method:" << log.method << std::endl;
    std::cout << "path:" << log.path << std::endl;
    std::cout << "protocol:" << log.protocol << std::endl;
    std::cout << "protocol_version:" << log.protocol_version << std::endl;
    std::cout << "\n\n" << std::endl;
}

int detection_loop(std::string log_path, SocketServer *sock_server)
{
    Reader reader;
    DEBUG_DECLARE(reader.reset_offset());

    Parser parser;
    Detector detector(50000, 5000, 30);
    Reporter reporter;

    while (true)
    {
        std::vector<std::string> log_lines = reader.read_logs(log_path);
        if (!reader.status)
            return 1;

        for (std::string &s : log_lines)
        {
            Log parsed_log = parser.parse(s);
            (detector.*detector.insert)(parsed_log); // insert is a pointer function switches between do_insert and process, to efficiently process logs
            int sz = detector.sus_log_counts(); // size of suspicious_logs
            if (sz > 0)
            {
                for (const std::pair<Log, Anomaly> sus_log : detector.get_sus_logs())
                {
                    std::string output = reporter.get_output(sus_log.first, sus_log.second);
                    sock_server->add_task(output);
                    DEBUG_LOG("readed line: " << s);
                }
                detector.reset();
            }
        }
    }

    return 0;
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
    std::thread server_thread(&SocketServer::serve, &sock_server);

    DEBUG_LOG("All logs are parsed and detector scanned!");
    server_thread.join();

    return 0;
}