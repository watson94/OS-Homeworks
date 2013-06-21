#include <iostream>
#include <pty.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stropts.h>
#include <poll.h>
#include <errno.h>

void  write_str (int  fd, char *str, int size) {
    int i = 0;
    while (i < size) {
        int write_res = write(fd, str + i, size - i);
        if (  write_res > 0) {
            i += write_res;
        }
        if (write_res == -1) {
            perror("Write error\n");
            exit(1);
         }
    }
}

void write_str(int fd, char * str) {
    write_str(fd, str, strlen(str));
}

int main() {
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    hints.ai_canonname = nullptr;
    hints.ai_addr = nullptr;
    hints.ai_next = nullptr;

    addrinfo *result;
    if (getaddrinfo(nullptr, "8322", &hints, &result) != 0)
    {
        std::exit(EXIT_FAILURE);
    }
    if (result == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
    int socket_fd;
    socket_fd = socket(result->ai_family, result->ai_socktype,
                        result->ai_protocol);
    connect(socket_fd, result->ai_addr, result->ai_addrlen);

    const int bufsize = 1024;
    char * res = (char * ) malloc(bufsize);
    if (res == NULL) {
        perror("Allocate error\n");
        return 2;
    }

    int rr = read(socket_fd, res, 1);
    if (rr == -1) {
        perror("Read error\n");
        return 3;
    }
 
    write(socket_fd, res, 1);
    int curlen = 0;
    while ((rr = read(socket_fd, res + curlen, bufsize)) > 0) {
        curlen += rr;
    }
    if (rr == -1) {
        perror("Read error\n");
        return 3;
    }

    int bufpointer = 0;
    bool flag = false;
    for (int i = 0; i < curlen ; i++) {
        if (res[i] == '\0') {
            if (!flag) {
                write_str(1, (char *) "server time: ");
                flag = true;
            } else {
                write_str(1, (char *) "ping: ");
            }
            write_str(1, res + bufpointer, i - bufpointer + 1);
            bufpointer = i + 1;
            write_str(1, (char *) "\n");
        }
    }
}
