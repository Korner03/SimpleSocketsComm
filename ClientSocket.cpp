//
// Created by cabby333 on 6/15/17.
//

#include <vector>
#include <algorithm>
#include "ClientSocket.h"

using namespace std;

ClientSocket::ClientSocket(int fd, string name, Server *server):
        fd(fd), name(name), server(server) {}

ClientSocket::~ClientSocket() {
    close(fd);
    fd = -1;
}

bool ClientSocket::read_write() {

    char buff[REQUEST_SIZE];

    if (read(fd, &buff, REQUEST_SIZE) == -1)
        print_error_and_exit("read");

    Request req;

    read_request(buff, req);

    Response res;

    switch (req.type) {

        case REQ_TYPES::EXIT:
            _exit(res);
            break;

        case REQ_TYPES::CREATE_GROUP:
            _create_group(req, res);
            break;

        case REQ_TYPES::SEND:
            _send(req, res);
            break;

        case REQ_TYPES::WHO:
            _who(res);
            break;

        default:
            res.type = RES_TYPES::FAIL;
            strcpy(res.msg, FAIL_MSG);
            break;
    }

    bool success = _send_response(res);

    if (req.type == REQ_TYPES::EXIT) { // suicide...
        Server *serv = server;
        close(fd);
        FD_CLR(fd, &serv->clientsFds);
        serv->names.erase(name);
        int fd_to_delete = fd;

        serv->open_sockets--;
        delete serv->clients[fd_to_delete];
        serv->clients[fd_to_delete] = nullptr;
        serv->clients.erase(fd_to_delete);
    }

    return success;
}

bool ClientSocket::_create_group(Request &req, Response &res) {

    if (server->names.count(req.name) == 1) {
        res.type = RES_TYPES::FAIL;
        sprintf(res.msg, MSG_CREATE_GROUP_ERROR_CLIENT, req.name);
        fprintf(stdout, MSG_CREATE_GROUP_ERROR_SERVER, name.c_str(), req.name);
        return false;
    }

    char* curr_client_name = strtok(req.msg, ",");

    set<int> temp;

    while (curr_client_name != NULL) {

        int fd_found = 0;

        for (auto &kvp: server->clients) {
            if (strcmp(kvp.second->name.c_str(), curr_client_name) == 0) {
                temp.insert(kvp.second->fd);
                fd_found += 1;
            }
        }

        if (fd_found == 0) {
            res.type = RES_TYPES::FAIL;
            sprintf(res.msg, MSG_CREATE_GROUP_ERROR_CLIENT, req.name);
            fprintf(stdout, MSG_CREATE_GROUP_ERROR_SERVER, name.c_str(), req.name);
            return false;
        }

        curr_client_name = strtok(NULL, ",");
    }

    if (temp.count(fd) == 0) {
        temp.insert(fd);
    }

    if (temp.size() < 2) {
        res.type = RES_TYPES::FAIL;
        sprintf(res.msg, MSG_CREATE_GROUP_ERROR_CLIENT, req.name);
        fprintf(stdout, MSG_CREATE_GROUP_ERROR_SERVER, name.c_str(), req.name);
        return false;
    }

    server->groups[req.name] = temp;

    for (auto item : temp) {
        server->groups[req.name].insert(item);
    }

    server->names.insert(req.name);
    res.type = RES_TYPES::SUCCESS;
    sprintf(res.msg, MSG_CREATE_GROUP_SUCCESS_CLIENT, req.name);
    fprintf(stdout, MSG_CREATE_GROUP_SUCCESS_SERVER, name.c_str(), req.name);
    return true;
}

bool ClientSocket::_send(Request &req, Response &res) {

    Response forward;

    forward.type = RES_TYPES::SUCCESS;
    sprintf(forward.msg, "%s: %s", name.c_str(), req.msg);

    strcpy(res.msg, MSG_SENT_SUCCESS_CLIENT);
    res.type = RES_TYPES::SUCCESS;

    if (server->groups.count(req.name) > 0) {

        if (server->groups[req.name].count(fd) > 0) {

            for (int curr_fd: server->groups[req.name]) {

                if (curr_fd == fd)
                    continue;

                send_to_fd(curr_fd, forward);
            }

            fprintf(stdout, MSG_SENT_SUCCESS_SERVER, name.c_str(), req.msg, req.name);
            return true;
        }
    }

    for (auto &kvp: server->clients) {
        if (kvp.second->name.compare(req.name) == 0) {
            send_to_fd(kvp.first, forward);
            fprintf(stdout, MSG_SENT_SUCCESS_SERVER, name.c_str(), req.msg, req.name);
            return true;
        }
    }

    res.type = RES_TYPES::FAIL;
    sprintf(res.msg, MSG_SENT_ERROR_CLIENT);
    fprintf(stdout, MSG_SENT_ERROR_SERVER, name.c_str(), req.msg, req.name);
    return false;
}

bool ClientSocket::_who(Response &res) {

    std::vector<string> tmp;

    for (auto &kvp : server->clients) {
        tmp.push_back(kvp.second->name);
    }

    std::sort(tmp.begin(), tmp.end());

    char* traveler = res.msg;

    for (unsigned int i = 0; i < tmp.size(); ++i) {

        memcpy(traveler, tmp[i].c_str(), tmp[i].size());

        traveler += tmp[i].size();
        if (i + 1 != tmp.size()) {
            *traveler = ',';
        } else {
            *traveler = '\0';
            break;
        }
        traveler++;
    }

    res.type = RES_TYPES::SUCCESS;
    fprintf(stdout, MSG_WHO_SUCCESS_SERVER, name.c_str());

    return true;
}

bool ClientSocket::_exit(Response &res) {

    server->names.erase(name);
    for (auto& kvp: server->groups) {
        kvp.second.erase(fd);
    }
    res.type = RES_TYPES::DIE;
    sprintf(res.msg, MSG_EXIT_SUCCESS_CLIENT);
    fprintf(stdout, MSG_EXIT_SUCCESS_SERVER, name.c_str());
    return true;
}




bool ClientSocket::_send_response(Response &res) {

    char buff[RESPONSE_SIZE] = {0};
    write_response(buff, res);

    if (write(fd, buff, RESPONSE_SIZE) == -1) {
        print_error_and_exit("write");
    }

    return true;
}

bool ClientSocket::send_to_fd(int target_fd, Response &res) {

    char buff[RESPONSE_SIZE];
    write_response(buff, res);

    if (write(target_fd, buff, RESPONSE_SIZE) == -1) {
        print_error_and_exit("write");
    }

    return true;
}