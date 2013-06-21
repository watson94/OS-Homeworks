#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#define UNUSED(x) (void * ) (x)

int main (int argc, char ** argv) {
    struct addrinfo * res;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    int s = getaddrinfo(NULL, "8722" , &hints, &res);
    if (s != 0) {
        perror("error getaddrinfo");
        return 1;
    }
    
    int sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd == -1) {
        printf("%i\n", s);
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
        printf("Imhere\n");
        perror("error listen");
        return 4;
    }


    while (1) {
        int cfd = accept(sfd, NULL, NULL);
        pid_t pid = fork();
        if (!pid) {
            //time
            //send 
            //get
            //
            dup2(cfd, 0);
            dup2(cfd, 1);
            dup2(cfd, 2);
            close(cfd);
            close(sfd);
            printf("Hello\n");
            exit(0);
        }
        close(cfd);
    }

    if ((argc < 1) || (argv == NULL)) {
        return 2;
    }
    printf("1\n");
    return 0;
}
