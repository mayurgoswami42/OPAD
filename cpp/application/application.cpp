#include "application.hpp"

std::vector<Application*> Application::instances = {};

Application::Application(int port, double max_req_speed, int sus_request_limit, int window_size): detector(window_size, sus_request_limit, max_req_speed), sock_server(port)
{
    start_time = utils::now();
    
    instances.emplace_back(this);
    std::signal(SIGINT, &Application::signal_handler); // std::signal expects a c style function, hence we have to use static member to pass
    std::signal(SIGTERM, &Application::signal_handler);
}

void Application::signal_handler(int signum)
{
    for (Application*& instance : instances)
        instance->stop();
}

void Application::detection_loop(const std::string& log_path)
{
    Parser parser("(date)(time)(log_type)(user_ip)(method)(path)(protocol)(version)(status)(message)", R"re((\d{4}-\d{2}-\d{2})\s+((?:\d{2}:){2}\d{2},\d{3})\s+(INFO|ERROR|WARN|FATAL)\s+((?:\d{1,3}\.){3}\d{1,3})\s+"(GET|POST|PUT|DELETE|PATCH)\s+(\S+)\s+((?:HTTP|HTTPS)/(\d(?:\.\d)?))"\s+(\d{3})\s+(\S+))re");
    Reporter reporter;

    utils::time start{};

    while (state_running)
    {
        start = utils::now();
        std::vector<std::string> log_lines = reader.read_logs(log_path);

        if (!reader.status)
        {
            DEBUG_PRINT("APPLICATION::ERROR -  Reader returned with bad status!");
            stop();
            return;
        }
        
        if (log_lines.size() == 0) // if traffic on server is not much then wait for some logs to collected
        {
            utils::thread_sleep(500);
            continue;
        }
        
        for (std::string &s : log_lines)
        {
            std::unordered_map<std::string, std::string> parsed_log = parser.parse(s);
            (detector.*detector.insert)(parsed_log); // insert is a pointer function switches between do_insert and process, to efficiently process logs
            int sz = detector.sus_log_counts();
            if (sz > 0)
            {
                for (const std::pair<Detector::Log, Anomaly> sus_log : detector.get_sus_logs())
                {
                    std::string output = reporter.get_output(sus_log.first, sus_log.second);
                    sock_server.add_task(output);
                    DEBUG_PRINT("APPLICATION::INFO - suspicious line: " << s);
                }
                detector.reset();
            }
        }

        tool_speed = log_lines.size()/((utils::duration)(utils::now() - start)).count();
    }
}

void Application::serve_forever()
{
    while (state_running)
        sock_server.serve();
}

void Application::additional_data() // share additional information about the engine
{
    double prev_tool_speed = 0;
    while (state_running)
    {
        if (prev_tool_speed == tool_speed)
        {
            prev_tool_speed = tool_speed;
            tool_speed = 0.0;
        }
        else prev_tool_speed = tool_speed;
        utils::duration uptime = utils::now() - start_time;
        std::string data = "{";
        data += detector.get_speed_snap();
        data += ",";
        data += utils::stringify("offset") + ":" + utils::stringify(reader.get_offset());
        data += ",";
        data += utils::stringify("uptime") + ":" + utils::stringify(uptime.count());
        data += ",";
        data += utils::stringify("tool_speed") + ":" + utils::stringify(tool_speed);
        data += "}";
        sock_server.add_task(data);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Application::run(const std::string &log_path)
{
    detection_thread = std::thread(&Application::detection_loop, this, log_path);
    server_thread = std::thread(&Application::serve_forever, this);
    data_thread = std::thread(&Application::additional_data, this);

    detection_thread.join();
    server_thread.join();
    data_thread.join();

    stop();
}

void Application::stop()
{
    state_running = false;
    sock_server.stop();
    reader.close();
}