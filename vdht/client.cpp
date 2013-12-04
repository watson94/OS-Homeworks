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


void  write_str (int  fd, const char *str, int size) {
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

int socket_fd;
const char * ident;
int option;
int facility;




char * getstring(long long x) {
    char * tmp = (char * ) malloc(1024);
    if (tmp == NULL) {
        perror("Alocate error\n");
        exit(6);
    }
    int k = 0;
    while(x > 0) {
        tmp[k] = '0' + (x % 10);
        x = x/10;
        k++;
    }
    char * ans = (char *) malloc((k + 1) * sizeof(char));
    if (ans == NULL) {
        perror("Alocating error\n");
        exit(6);
    }
    for (int i = 0; i < k; i++) {
        ans[i] = tmp[k - i - 1];
    }
    ans[k] = '\0';
    free(tmp);
    return ans;
}




void openlog(const char *_ident, int _option, int _facility) {
    ident = _ident;
    option = _option;
    facility = _facility;

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
    if (getaddrinfo(nullptr, "8922", &hints, &result) != 0)
    {
        std::exit(EXIT_FAILURE);
    }
    if (result == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
    socket_fd = socket(result->ai_family, result->ai_socktype,
                        result->ai_protocol);
    connect(socket_fd, result->ai_addr, result->ai_addrlen);
}
void syslog(int priority, const char *str) {
    write_str(socket_fd, ident, strlen(ident));
    write_str(socket_fd, " ", 1);
    write_str(socket_fd, str, strlen(str));
    write_str(socket_fd, " ", 1);
    char * pr = getstring(priority);
    write_str(socket_fd, pr);
    write_str(socket_fd, "\n",  1);
}

void closelog() {
}

