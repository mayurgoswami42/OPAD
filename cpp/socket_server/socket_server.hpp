#pragma once

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>

#include "utils/debug.hpp"

class SocketServer
{
public:
    SocketServer(int port);
    std::mutex socket_mutex;
    std::condition_variable socket_cv;
    void serve();
    void add_task(std::string task);
    void start(int clients);
    void stop();

private:
    std::queue<std::string> task_queue;
    int server_fd;
    const int _port;
    std::vector<int> clients_fds;
    std::atomic<bool> server_running{true};
    sockaddr_in server_addr{};
    
    void remove_client(int clinet_index);
    void send_msg(int client_fd, std::string message);
    std::optional<std::string> receive(int client_fd);
};