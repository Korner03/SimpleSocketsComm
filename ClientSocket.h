//
// Created by cabby333 on 6/15/17.
//

#ifndef EX5_CLIENTSOCKET_H
#define EX5_CLIENTSOCKET_H

#include <string>
#include "whatsappServer.h"
#include "whatsappProtocol.h"

typedef class Server Server;

class ClientSocket {
public:
    ClientSocket() = delete;
    ClientSocket(int fd, std::string name, Server* server);
    ~ClientSocket();
    bool read_write();

    static bool send_to_fd(int fd, Response &res);
    int fd;
    std::string name;

private:
    Server* server;

    bool _create_group(Request &req, Response &res);
    bool _send(Request &req, Response &res);
    bool _who(Response &res);
    bool _exit(Response &res);
    bool _send_response(Response &res);

};


#endif //EX5_CLIENTSOCKET_H
