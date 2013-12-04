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
#include <fcntl.h>
#include <mutex>
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
int main (int argc, char ** argv) {
    if (argc != 2) {
        perror("Incorrect number of arguments");
        return 1; 
    }
    if (( pid = fork())) {
        signal(SIGINT, &handler);
        wait(NULL);
        return 0;
    } else {
        int outfd = open(argv[1] , O_CREAT | O_RDWR, S_IRWXU);
        struct flock flock;
        flock.l_type = F_WRLCK;
        flock.l_whence = SEEK_CUR;
        flock.l_start = 1;
        flock.l_len = 1;
        const int buffersize = 4096;
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
        int s = getaddrinfo(NULL, "8922" , &hints, &res);
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
        printf("Imhere\n");
        while (1) {
	    int cfd = accept(sfd, NULL, NULL);
            printf("Imhere1\n");
            if (cfd == -1) {
                perror("Accept error\n");
                return(9);
            }
            pid_t pid = fork();
            if (!pid) {
                char * buffer = (char *) malloc(buffersize);
                if (buffer == NULL) {
                    return 7;
                }
                while (1) {
                    int r = read(cfd, buffer, buffersize);
                    if (r < 0) {
                        break;
                    } 
                    if (r == 0) {
                        break;
                    }
                    flock.l_pid =  pid;
                    fcntl(outfd, F_SETLK, &flock);
                    write_str(outfd, buffer, r); 
                    flock.l_type = F_UNLCK;
                    fcntl(outfd, F_SETLK ,&flock);
                }
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
