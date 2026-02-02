#ifndef SERVER_H
#define SERVER_H

#include "thread_pool.h"
#include <atomic>

class Server {
public:
    Server(int port);
    void start();
    void stop();

private:
    int port;
    int serverFd;
    int epollFd;

    std::atomic<bool> running;
    std::atomic<int> activeClients;

    ThreadPool pool;

    int createServerSocket();
    void handleClient(int clientFd);
};

#endif

