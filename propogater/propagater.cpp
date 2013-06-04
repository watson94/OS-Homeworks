#include <stdlib.h>

#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <memory.h>
const int buffersize = 4096;

struct state {
    char * buffer;
    bool isalive;
    bool iseof;
    int len;
    state () {
        len = 0;
        iseof = false;
        isalive = true;
        buffer = (char *) malloc(buffersize);
    };
};

int main(int argc, char ** argv) {
    if (argc < 3) {
        printf("There are should be ar least 2 arguments");
        return 1; 
    } 
    if (argc % 2 != 1) {
        printf("Number of argument must be even\n");
        return 2;
    }
    struct pollfd fds[argc - 1];
//  printf("%s\n", "Im in propogater");
    for (int i = 1; i < argc; i++) {
//      printf("%s\n", argv[i]);
        if (i % 2 == 1) {
            fds[i - 1].fd = atoi(argv[i]) ;
            fds[i - 1].events = POLLIN | POLLERR;

        } else {
            fds[i - 1].fd = atoi(argv[i]) ;
            fds[i - 1].events = POLLOUT | POLLERR;
        }
    }
    int n = argc - 1;
    bool myeof = false;
    struct state state[n/2];
    while (!myeof) {
        poll(fds, n, -1 );
        myeof = true;
        for (int j = 0; j < n/2; j++) {
            if (state[j].isalive == true) {
                myeof = false;
                int i = j * 2;
                if ((fds[i].revents & POLLERR) || (fds[i].revents & POLLERR)) {
                    state[j].isalive = false;
                }
                if (state[j].len < buffersize) {
                    if ((!state[j].iseof) && (fds[i].revents & POLLIN)) {
//                      printf("Im reading\n");
                        int r = read(fds[i].fd, state[j].buffer + state[j].len, buffersize - state[j].len);
                        if (r == 0) {
                           state[j].iseof = true; 
                        } else {
                          state[j].len += r;
                        }
                    }
                }
//              printf("%i\n", i);
//              printf("%i %i\n", fds[i].fd, state[j].len);
                if ((state[j].len == 0) && (state[j].iseof == true)) {
                    state[j].isalive = false;
                }
                if (state[j].len > 0) {
                    if (fds[i + 1].revents & POLLOUT) {
                        int w = write(fds[i + 1].fd, state[j].buffer, state[j].len);
                        memmove(state[j].buffer, state[j].buffer + w, state[j].len - w);
                        state[j].len -= w;
                    }
                }
            }
        }
    }
    return 0;
}
