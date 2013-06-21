#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>

void write_str (int  fd, char *str, int size) {
    int i = 0;
    while (i < size) {
        int write_res = write(fd, str + i, size - i);
        if (  write_res > 0) {
            i += write_res;
        }
        if (write_res == -1) {
            perror("Write error\n");
            exit(8);
        }
    }
}

void write_str(int fd, char * str) {
    write_str(fd, str, strlen(str));
}

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

int pid;
void handler(int) {
    if (pid) {
        kill(pid, SIGINT);
    }
}
int main () {
    const long long MIL = 1000000000;
    if (( pid = fork())) {
        signal(SIGINT, &handler);
        wait(NULL);
        return 0;
    } else {
        struct addrinfo * res;
        struct addrinfo hints;
        setsid();

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
        hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
        hints.ai_protocol = 0;          /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;
        int s = getaddrinfo(NULL, "833322" , &hints, &res);
        if (s != 0) {
            perror("error getaddrinfo");
            return 1;
        }
    
        int sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sfd == -1) {
            perror("error socket");
            return 2;
        }
        int optval = 1;
        setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));
    
        if ( (bind(sfd, res->ai_addr, res->ai_addrlen)) == -1) {
            perror("error bind");
            return 3;
        } 
        if (listen(sfd, 5) == -1) {
            perror("error listen");
            return 4;
        }

        while (1) {
            int cfd = accept(sfd, NULL, NULL);
            if (cfd == -1) {
                perror("Accept error\n");
                return(9);
            }
            pid_t pid = fork();
            if (!pid) {
                struct timespec start;
                int rr = clock_gettime(CLOCK_MONOTONIC_RAW, &start);
                if (rr == -1) {
                    perror("Clock error");
                    return 5;
                }
                write(cfd, "Z", 1);
                char * res = (char * ) malloc(10);
                if (res == NULL) {
                    perror("Allocate error");
                    return 6;
                }

                rr = read(cfd, res, 1);
                if (rr == -1) {
                    perror("Read error");
                    return 7;
                }

                struct timespec finish;

                rr = clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
                if (rr == -1) {
                    perror("Clock error");
                    return 5;
                }


                long long ftime = (finish.tv_sec * MIL + finish.tv_nsec);
                long long stime = (start.tv_sec * MIL + start.tv_nsec); 

                write_str(1, (char *) "server time: ");
                
                char * arg1 = getstring(ftime);
                char * arg2 = getstring(ftime - stime);
                write_str(1, arg1);
                write_str(1, (char *) "\n");

                write_str(cfd, arg1, strlen(arg1) + 1);
                write_str(cfd, arg2, strlen(arg2) + 1);
                
                //send 
                //get
                //time
                //sendtime 
                /*
                dup2(cfd, 0);
                dup2(cfd, 1);
                dup2(cfd, 2);
                close(cfd);
                close(sfd);
                printf("Hello\n"); */
                exit(0);
            }
            close(cfd);
        }
        printf("1\n");
        return 0;
    }
}
