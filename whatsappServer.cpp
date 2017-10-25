//
// Created by cabby333 on 6/15/17.
//

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include "whatsappServer.h"

#define SERVER_USAGE "Usage: whatsappServer portNum\n"
#define MAX_MSG_LEN 256


//132.65.125.119
using namespace std;

/* Server */
Server::Server(int port): open_sockets(0), port(port), server_socket(0), max_fd(0) {}

Server::~Server() {
    for (auto& kvp: clients) {

        if (kvp.second != nullptr) {
            Response kill_message;
            kill_message.type = RES_TYPES::DIE;
            strcpy(kill_message.msg, DISCONNECTED_BY_SERVER);

            kvp.second->send_to_fd(kvp.first, kill_message);
            close(kvp.second->fd);

            delete(kvp.second);
            kvp.second = nullptr;
        }
    }

    clients.clear();
    groups.clear();
    names.clear();

    close(server_socket);
    server_socket = -1;
    max_fd = -1;
}

bool Server::run() {

    char myname[MAX_MSG_LEN + 1];

    // set lister
    struct sockaddr_in sa_me;
    struct hostent* hp;

    if (gethostname(myname, MAX_MSG_LEN) == -1)
        print_error_and_exit("gethostname");

    if ((hp = gethostbyname(myname)) == NULL)
        print_error_and_exit("gethostbyname");

    memset((char*) &sa_me, 0, sizeof(struct sockaddr_in));
    sa_me.sin_family = (sa_family_t) hp->h_addrtype;
    memcpy(&sa_me.sin_addr, hp->h_addr, hp->h_length);
//    cerr << inet_ntoa(sa_me.sin_addr) << endl;
    sa_me.sin_port = htons((uint16_t) port);


    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        print_error_and_exit("socket");
    }

    if (bind(server_socket, (struct sockaddr*) &sa_me, sizeof(struct sockaddr_in)) < 0) {
        close(server_socket);
        return false;
    }

    listen(server_socket, MAX_PENDING);
    max_fd = server_socket;
    fd_set readFds;

    FD_ZERO(&clientsFds);
    FD_SET(server_socket, &clientsFds);
    FD_SET(STDIN_FILENO, &clientsFds);

    while (true) {
        readFds = clientsFds;

        if (select(max_fd + 1, &readFds, NULL, NULL, NULL) < 0) {
            print_error_and_exit("select");
        }

        if (FD_ISSET(STDIN_FILENO, &readFds)) {
            char input[MAX_MSG_LEN];

            read_from_stdin(STDIN_FILENO, input);
            if (strcmp(input, "EXIT\n") == 0) {
                cout << SERVER_SHUTDOWN << endl;
                return terminate();
            }

        } else if (FD_ISSET(server_socket, &readFds)) { // connect new client

            socklen_t sock_add_len = sizeof(struct sockaddr_in);
            int new_fd = accept(
                    server_socket,
                    (struct sockaddr*) &sa_me,
                    &sock_add_len
            );

            if (new_fd < 0) {
                print_error_and_exit("accept");
            }

            string name;

            char buff[REQUEST_SIZE];
            if (read(new_fd, buff, REQUEST_SIZE) == -1)
                print_error_and_exit("read");

            Request req;
            read_request(buff, req);

            if (req.type != REQ_TYPES::CONNECT) {

                Response res;
                res.type = RES_TYPES::FAIL;
                strcpy(res.msg, FAIL_MSG);

                ClientSocket::send_to_fd(new_fd, res);
                fprintf(stdout, SERVER_FAILED_CONNECT, "UNKNOWN");

                close(new_fd);
                continue;
            }

            // no free sockets
            else if (open_sockets == 30) {

                Response res;
                res.type = RES_TYPES::FAIL;
                strcpy(res.msg, FAIL_MSG);

                ClientSocket::send_to_fd(new_fd, res);

                close(new_fd);
                fprintf(stdout, SERVER_FAILED_CONNECT, req.name);

                continue;

            } else {// there is an available socket

                name = req.name;

                // client name exist;
                if (names.count(name) > 0) {

                    Response res;
                    res.type = RES_TYPES::FAIL;
                    strcpy(res.msg, USAGE_CLIENT_EXIST_FAIL);

                    ClientSocket::send_to_fd(new_fd, res);
                    fprintf(stdout, SERVER_FAILED_CONNECT, req.name);

                    close(new_fd);
                    continue;

                } else { // client name does not exist

                    Response res;
                    res.type = RES_TYPES::SUCCESS;
                    strcpy(res.msg, USAGE_CLIENT_SUCCESS);
                    fprintf(stdout, SERVER_CONNECTED, req.name);
                    ClientSocket::send_to_fd(new_fd, res);
                }
            }

            open_sockets++;

            clients[new_fd] = new ClientSocket(new_fd, name, this);

            if (new_fd > max_fd)
                max_fd = new_fd;
            names.insert(name);

            FD_SET(new_fd, &clientsFds);


        } else {

            for ( auto& kvp : clients) {
                if (FD_ISSET(kvp.first, &readFds)) {
                    kvp.second->read_write();
                    break;
                }
            }
        }

    }
}


bool Server::terminate() {
    for (auto& kvp: clients) {

        if (kvp.second != nullptr) {
            Response kill_message;
            kill_message.type = RES_TYPES::DIE;
            strcpy(kill_message.msg, DISCONNECTED_BY_SERVER);

            kvp.second->send_to_fd(kvp.first, kill_message);
            close(kvp.second->fd);

            delete(kvp.second);
            kvp.second = nullptr;
        }
    }

    clients.clear();
    groups.clear();
    names.clear();

    close(server_socket);
    server_socket = -1;
    max_fd = -1;

    exit(0);
}

int main(int argc, char** argv) {

    if (argc != 2) {
        cout << SERVER_USAGE;
        return 1;
    }

    // parse input
    int port;
    char temp[100];
    int res = sscanf(argv[1], "%d%s\n", &port, temp);

    if (res != 1) {
        cout << SERVER_USAGE;
        return 1;
    }

    Server server(port);
    if (server.run())
        return 1;

    return 0;
}