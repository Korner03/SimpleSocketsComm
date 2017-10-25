//
// Created by cabby333 on 6/15/17.
//

#include <iostream>
#include "whatsappProtocol.h"

using namespace std;

void read_request(char *buff, Request &req) {

    char type = buff[0];

    if (type > REQ_TYPES::NUM) {
        req.type = REQ_TYPES::NOT_EXIST;
    } else {
        req.type = (REQ_TYPES) type;
    }

    memcpy(req.name, &buff[1], NAME_SIZE);
    memcpy(req.msg, &buff[31], MSG_SIZE);

}

void write_request(char *buff, Request &req) {
    if (buff == nullptr) {
        return;
    }
    buff[0] = req.type;
    memcpy(&buff[1], req.name, NAME_SIZE);
    memcpy(&buff[31], req.msg, MSG_SIZE);
}

void read_response(char *buff, Response &res) {

    char type = buff[0];

    if (type > RES_TYPES::RES_NUM) {
        res.type = REQ_TYPES::NOT_EXIST;
    } else {
        res.type = (RES_TYPES) type;
    }

    memcpy(res.msg, &buff[1], RESPONSE_SIZE - 1);

}

void write_response(char *buff, Response &res) {
    if (buff == nullptr) {
        return;
    }
    buff[0] = res.type;
    memcpy(&buff[1], res.msg, RESPONSE_SIZE - 1);

}

int read_data(int curr_fd) {

    char buffer[RESPONSE_SIZE + 1];
    int read_size = 0;

    read_size = (int) read(
            curr_fd,
            buffer,
            (size_t) (RESPONSE_SIZE + 1)
    );

    if (read_size == -1)
        print_error_and_exit("read");

    Response res;
    read_response(buffer, res);
    cout << res.msg << endl;

    if (res.type == RES_TYPES::DIE) {
        exit(0);
    }

    while (read_size > RESPONSE_SIZE) {
        buffer[0] = buffer[RESPONSE_SIZE];

        read_size = (int) read(
                curr_fd,
                buffer + 1,
                (size_t) (RESPONSE_SIZE)
        );

        if (read_size == -1)
            print_error_and_exit("read");

        read_response(buffer, res);
        cout << res.msg << endl;

        if (res.type == RES_TYPES::DIE) {
            exit(0);
        }
    }

    return read_size;

}


int read_from_stdin(int curr_fd, char* buff) {
    int bytes_read = 0;
    int curr_bytes_read = 0;

    while (bytes_read < REQUEST_SIZE) {
        curr_bytes_read = (int) read(curr_fd, buff + bytes_read, 1);

        if (curr_bytes_read == -1)
            print_error_and_exit("read");

        if (curr_bytes_read > 0) {
            bytes_read += curr_bytes_read;
        }

        if (buff[bytes_read - 1] == '\n') {
            buff[bytes_read] = '\0';
            break;
        }
    }

    return bytes_read;
}

void print_error_and_exit(string function_name) {
    printf("ERROR: %s %d.\n", function_name.c_str(), errno);
    exit(1);
}