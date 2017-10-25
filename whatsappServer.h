//
// Created by cabby333 on 6/15/17.
//

#ifndef EX5_WHATSAPPSERVER_H
#define EX5_WHATSAPPSERVER_H

#include <zconf.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <netdb.h>
#include <iostream>
#include <map>
#include <set>
#include "ClientSocket.h"

#define MAX_PENDING 10
#define EXIT_MSG "EXIT command is typed: server is shutting down"
#define FAIL_MSG "ERROR\n"

typedef class ClientSocket ClientSocket;


class Server {
public:
    Server() = delete;
    Server(int port);
    ~Server();

    bool run();
    bool terminate();

    std::map<int, ClientSocket*> clients;
    std::map<std::string, std::set<int>> groups;
    std::set<std::string> names;
    fd_set clientsFds;
    int open_sockets;

private:
    int port;
    int server_socket;

    int max_fd;
};

#endif //EX5_WHATSAPPSERVER_H
