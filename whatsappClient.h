//
// Created by cabby333 on 6/15/17.
//

#ifndef EX5_WHATSAPPCLIENT_H
#define EX5_WHATSAPPCLIENT_H

#include <iostream>
#include "whatsappProtocol.h"

class Client {
public:
    Client() = delete;
    Client(char* name, char* ip, int port);
    ~Client();

    bool run();
    bool check_name(char* in_name);
    bool check_data(Request& req, char* buff);

private:
    int port;
    char* name;
    char* ip;
};

#endif //EX5_WHATSAPPCLIENT_H
