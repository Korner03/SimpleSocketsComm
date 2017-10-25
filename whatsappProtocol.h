//
// Created by cabby333 on 6/15/17.
//

#ifndef EX5_WHATSAPPPROTOCOL_H
#define EX5_WHATSAPPPROTOCOL_H

#include <stdlib.h>
#include <string.h>
#include <zconf.h>
#include <errno.h>

#define NAME_SIZE 30
#define MSG_SIZE 993
#define REQUEST_SIZE 1 + NAME_SIZE + MSG_SIZE
#define RESPONSE_SIZE 1024

#define MSG_CREATE_GROUP_SUCCESS_SERVER "%s: Group \"%s\" was created successfully.\n"
#define MSG_CREATE_GROUP_SUCCESS_CLIENT "Group \"%s\" was created successfully."
#define MSG_CREATE_GROUP_ERROR_SERVER "%s: failed to create group \"%s\".\n"
#define MSG_CREATE_GROUP_ERROR_CLIENT "ERROR: failed to create group \"%s\"."

#define MSG_SENT_SUCCESS_SERVER "%s: \"%s\" was sent successfully to \"%s\".\n"
#define MSG_SENT_SUCCESS_CLIENT "Sent Successfully. "
#define MSG_SENT_ERROR_SERVER "%s: ERROR: failed to send \"%s\" to %s.\n"
#define MSG_SENT_ERROR_CLIENT "ERROR: failed to send."

#define MSG_WHO_SUCCESS_SERVER "%s: Requests the currently connected client names.\n"
#define MSG_WHO_ERROR_CLIENT "ERROR: failed to receive list of connected clients."

#define MSG_EXIT_SUCCESS_SERVER "%s: Unregistered successfully.\n"
#define MSG_EXIT_SUCCESS_CLIENT "Unregistered successfully."

#define USAGE_CLIENT_SUCCESS "Connected Successfully."
#define SERVER_CONNECTED "%s connected.\n"
#define SERVER_FAILED_CONNECT "%s failed to connect.\n"
#define USAGE_CLIENT_ERROR "Failed to connect the server"
#define USAGE_CLIENT_EXIST_FAIL "Client name is already in use."

#define INVALID_USER_INPUT "ERROR: Invalid input."
#define SERVER_SHUTDOWN "EXIT command is typed: server is shutting down."
#define DISCONNECTED_BY_SERVER "Disconnected by server."



enum REQ_TYPES : char {
    CONNECT = 0,
    EXIT = 1,
    CREATE_GROUP = 2,
    SEND = 3,
    WHO = 4,
    NUM = 5,
    NOT_EXIST = 127,
};

typedef struct Request {
    Request() {
        type = REQ_TYPES::NOT_EXIST;

        for (int i = 0; i < NAME_SIZE - 1; i++) {
            name[i] = '\0';
        }

        for (int i = 0; i < MSG_SIZE - 1; i++) {
            msg[i] = '\0';
        }
    }

    char type;
    char name[NAME_SIZE];
    char msg[MSG_SIZE];
} Request;

enum RES_TYPES : char {
    SUCCESS = 0,
    FAIL = 1,
    DIE = 2,
    RES_NUM = 3,
    NOT_EXISTS = 127
};

typedef struct Response {
    Response() {
        type = RES_TYPES::NOT_EXISTS;

        for (int i = 0; i < RESPONSE_SIZE - 1; i++) {
            msg[i] = '\0';
        }
    }

    char type;
    char msg[RESPONSE_SIZE - 1];
} Response;

void read_request(char *buff, Request &req);
void write_request(char *buff, Request &req);

void read_response(char *buff, Response &res);
void write_response(char *buff, Response &res);

int read_data(int curr_fd);
int read_from_stdin(int curr_fd, char* buff);

void print_error_and_exit(std::string function_name);
#endif //EX5_WHATSAPPPROTOCOL_H
