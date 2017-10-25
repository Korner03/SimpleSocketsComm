//
// Created by cabby333 on 6/15/17.
//
#include <regex>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include "whatsappClient.h"

#define USAGE_CLIENT "whatsappClient clientName serverAddress serverPort\n";

using namespace std;

Client::Client(char* name, char* ip, int port): port(port), name(name), ip(ip) {}

Client::~Client() {}

bool Client::check_name(char* in_name) {
    return std::regex_match(in_name, std::regex("^[0-9a-zA-Z]+$"));
}

bool Client::check_data(Request& req, char* buff) {

    // create group
    if (regex_match(buff, regex(
            "^(create_group)( )([0-9a-zA-Z]+( )[0-9a-zA-Z]+([,][0-9a-zA-Z]+)*)\n$"))) {

        req.type = REQ_TYPES::CREATE_GROUP;

        int name_start = 13;
        int name_end = 0;
        int msg_end = 0;

        for (int i = name_start; i < REQUEST_SIZE; i++) {
            if (name_end == 0 && buff[i] == ' ')
                name_end = i;

            if (buff[i] == '\n') {
                msg_end = i;
                break;
            }
        }

        memcpy(req.name, &buff[name_start], (size_t) (name_end - name_start));
        req.name[name_end - name_start] = '\0';
        memcpy(req.msg, &buff[name_end + 1], (size_t) (msg_end - name_end + 1));
        req.msg[msg_end - name_end - 1] = '\0';

    // send
    } else if (regex_match(buff, regex("^(send)( )([0-9a-zA-Z]+)( )(.+)\n$"))) {
        req.type = REQ_TYPES::SEND;

        int name_start = 5;
        int name_end = 0;
        int msg_end = 0;

        for (int i = name_start; i < REQUEST_SIZE; i++) {
            if (name_end == 0 && buff[i] == ' ')
                name_end = i;

            if (buff[i] == '\n') {
                msg_end = i;
                break;
            }
        }

        memcpy(req.name, &buff[name_start], (size_t) (name_end - name_start));
        req.name[name_end - name_start] = '\0';
        memcpy(req.msg, &buff[name_end + 1], (size_t) (msg_end - name_end + 1));
        req.msg[msg_end - name_end - 1] = '\0';


    // who
    } else if (regex_match(buff, regex("^(who)\n$"))) {
        req.type = REQ_TYPES::WHO;

    // exit
    } else if (regex_match(buff, regex("^(exit)\n$"))) {
        req.type = REQ_TYPES::EXIT;

    } else {
        return false;
    }

    return true;
}

bool Client::run() {

    if (!check_name(name)) {
        cout << USAGE_CLIENT_ERROR << endl;
        return false;
    }

    // create the socket
    struct sockaddr_in sa;
    struct hostent* hp;
    int client_socket;

    if ((hp = gethostbyname(ip)) == NULL) {
        print_error_and_exit("gethostbyname");
    }

    memset((char*) &sa, 0, sizeof(struct sockaddr_in));
    memcpy((char*) &sa.sin_addr, hp->h_addr, (size_t) hp->h_length);
    sa.sin_family = (sa_family_t) hp->h_addrtype;
    sa.sin_port = htons((u_short) port);

    // create socket
    if ((client_socket = socket(hp->h_addrtype, SOCK_STREAM, 0)) < 0) {
        print_error_and_exit("write");
    }
    setsockopt(client_socket, IPPROTO_TCP, TCP_NODELAY, 0, sizeof(int));

    fd_set readFds;

    // connect to server
    int ss = connect(client_socket, (struct sockaddr*) &sa, sizeof(sa));
    if (ss < 0) { // connect fail
        close(client_socket);
        print_error_and_exit("connect");
    } else { // connect success
        char connected_msg[REQUEST_SIZE];
        for (int i = 0; i < REQUEST_SIZE; i++) {
            connected_msg[i] = '\0';
        }
        Request req;

        req.type = REQ_TYPES::CONNECT;
        memcpy(req.name, name, NAME_SIZE);
        write_request(connected_msg, req);

        if (write(client_socket, connected_msg, (size_t) REQUEST_SIZE - 1) == -1)
            print_error_and_exit("write");
        if (read(client_socket, connected_msg, (size_t) RESPONSE_SIZE) == -1)
            print_error_and_exit("read");

        Response res;
        read_response(connected_msg, res);

        cout << res.msg << endl;
        if (res.type != RES_TYPES::SUCCESS) {
            exit(1);
        }
    }


    while (true) {

        FD_ZERO(&readFds);
        FD_SET(client_socket, &readFds);
        FD_SET(STDIN_FILENO, &readFds);

        if (select(client_socket + 2, &readFds, NULL, NULL, NULL) < 0) {
            print_error_and_exit("select");
        }

        if (FD_ISSET(STDIN_FILENO, &readFds)) { // user entered a msg
            char user_input[REQUEST_SIZE];
            int bytes_read = read_from_stdin(STDIN_FILENO, user_input); // get data from user

            if (bytes_read == 0)
                continue;

            Request req;

            if (!check_data(req, user_input)) { // check the msg is in valid format

                string user_input_string = user_input;

                if (user_input_string.find("who ") == 0) {
                    cout << MSG_WHO_ERROR_CLIENT << endl;

                } else if (user_input_string.find("create_group ") == 0) {
                    char group_name[NAME_SIZE] = {0};
                    sscanf(user_input, "create_group %s", group_name);
                    fprintf(stdout, MSG_CREATE_GROUP_ERROR_CLIENT, group_name);
                    fprintf(stdout, "\n");

                }  else {
                    cout << INVALID_USER_INPUT << endl;
                }

            } else {
                write_request(user_input, req);

                if (write(client_socket, user_input, (size_t) REQUEST_SIZE) == -1)
                    print_error_and_exit("write");

                read_data(client_socket);

            }

        } else if (FD_ISSET(client_socket, &readFds)) { // server responded

            int bytes_read = read_data(client_socket);

            if (bytes_read == 0)
                continue;

        }

    }

}

int main(int argc, char** argv) {

    if (argc != 4) {
        cout << USAGE_CLIENT;
        return 1;
    }

    // parse input
    char *client_name = argv[1];
    char *server_address = argv[2];
    int server_port = 0;

    char temp[100];
    int res = sscanf(argv[3], "%d%s\n", &server_port, temp);

    if (res != 1) {
        cout << USAGE_CLIENT;
        return 1;
    }

    Client client(client_name, server_address, server_port);

    if (!client.run())
        return 1;

    return 0;
}