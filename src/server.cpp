#include "server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <iostream>

#define MAX_EVENTS 1024
#define BUFFER_SIZE 4096

static Server* globalServer = nullptr;

void handleSigInt(int) {
    if (globalServer) {
        globalServer->stop();
    }
}

Server::Server(int port)
    : port(port),
      running(true),
      activeClients(0),
      pool(4) {}

int Server::createServerSocket() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(fd, (sockaddr*)&addr, sizeof(addr));
    listen(fd, SOMAXCONN);

    fcntl(fd, F_SETFL, O_NONBLOCK);
    return fd;
}

void Server::handleClient(int clientFd) {
    char buffer[BUFFER_SIZE];
    std::string request;

    while (true) {
        int bytes = read(clientFd, buffer, BUFFER_SIZE);
        if (bytes <= 0) break;

        request.append(buffer, bytes);

        // End of HTTP headers
        if (request.find("\r\n\r\n") != std::string::npos)
            break;
    }

    if (request.empty()) {
        close(clientFd);
        activeClients--;
        return;
    }

    if (request.rfind("GET ", 0) == 0) {
        std::string body = "Hello from TCP Server";

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "Connection: close\r\n"
            "\r\n" +
            body;

        write(clientFd, response.c_str(), response.size());
    } else {
        std::string response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Connection: close\r\n"
            "\r\n";

        write(clientFd, response.c_str(), response.size());
    }

    close(clientFd);
    activeClients--;
}

void Server::start() {
    globalServer = this;
    signal(SIGINT, handleSigInt);

    serverFd = createServerSocket();
    epollFd = epoll_create1(0);

    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = serverFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &event);

    epoll_event events[MAX_EVENTS];

    std::cout << "Server running on port " << port << std::endl;

    while (running) {
        int nfds = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == serverFd) {
                int clientFd = accept(serverFd, nullptr, nullptr);
                fcntl(clientFd, F_SETFL, O_NONBLOCK);

                epoll_event ev{};
                ev.events = EPOLLIN;
                ev.data.fd = clientFd;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev);

                activeClients++;
                std::cout << "Client connected | Active clients: "
                          << activeClients.load() << std::endl;
            } else {
                int clientFd = events[i].data.fd;
                pool.enqueue([this, clientFd]() {
                    handleClient(clientFd);
                });
            }
        }
    }

    close(serverFd);
    close(epollFd);
    std::cout << "\nServer shut down gracefully\n";
}

void Server::stop() {
    running = false;
}

