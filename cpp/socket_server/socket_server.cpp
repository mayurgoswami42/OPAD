#include "socket_server.hpp"

SocketServer::SocketServer(int port) : _port(port)
{
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) DEBUG_LOG("Failed to create socket!");

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
}

void SocketServer::remove_client(int client_index)
{
    if (clients_fds.empty() || client_index < 0 || client_index >= clients_fds.size()) return;
    if (client_index + 1 == clients_fds.size())
    {
        clients_fds.pop_back();
        return;
    }
    int temp = clients_fds.back();
    clients_fds.pop_back();
    clients_fds[client_index] = temp;
}

void SocketServer::serve()
{
    start(1); // control clients count, increase as needed
    while (true)
    {
        std::unique_lock lock(socket_mutex);

        socket_cv.wait(lock, [this] {
            return !task_queue.empty();
        });
        
        std::string msg = task_queue.front();
        for (int i = 0; i < clients_fds.size(); ++i)
        {
            send_msg(clients_fds[i], msg);
            auto rec_msg = receive(clients_fds[i]);
            if (!rec_msg.has_value())
            {
                DEBUG_LOG("Client " << clients_fds[i] << " is disconnected");
                remove_client(i);
                if (clients_fds.size() <= 0)
                {
                    stop();
                    return;
                }
            }
        }
        task_queue.pop();
    }
}

void SocketServer::add_task(std::string task)
{
    {
        std::lock_guard lock(socket_mutex);
        task_queue.push(task);
    }
    socket_cv.notify_one(); // change with .notify_all() in case of multiple clients
}

void SocketServer::send_msg(int client_fd, std::string message)
{
    uint32_t len = htonl(static_cast<uint32_t>(message.size()));
    send(client_fd, &len, sizeof(len), 0);
    send(client_fd, message.c_str(), message.size(), 0);
}

std::optional<std::string> SocketServer::receive(int client_fd)
{
    char buffer[1024] = {0};
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n <= 0) return std::nullopt;
    return std::string(buffer, n);
}

void SocketServer::start(int clients = 0)
{
    listen(server_fd, clients);
    DEBUG_LOG("Waiting for the connection on the port: " << _port);
    int client_fd = accept(server_fd, nullptr, nullptr);
    clients_fds.emplace_back(client_fd);
    DEBUG_LOG("Client " << client_fd << " is connected! Total clients: " << clients_fds.size());
}

void SocketServer::stop()
{
    for (const int &client_fd : clients_fds)
    {
        close(client_fd);
    }
    close(server_fd);
}