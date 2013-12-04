#include <iostream>
#include <vector>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <utility>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>


#define UNUSED(x) (void ) (x)
using namespace std;



void write_str(int fd, char * str, int size) {
    int i = 0;
    while (i < size) {
        int write_res = write(fd, str + i, size - i);
        if( write_res > 0) {
            i+= write_res;
        }
    }
}


sig_atomic_t sbreak = 0, sprint = 0;

void h1(int signal) {
    sbreak = 1;
    UNUSED(signal);
}

void h2(int signal) {
    sprint = 1;
    UNUSED(signal);
}


int main(int argc, char ** argv) {
    UNUSED(argc);
    int buf_size = atoi(argv[1]);
    char * f1 = argv[2];
    char * f2 = argv[3];
    signal(SIGINT, h1);
    signal(SIGUSR1, h2);

    int d1 = open(f1, O_RDONLY, S_IRWXU);
    int d2 = open(f2, O_WRONLY | O_CREAT, S_IRWXU);
    
    char * buffer = (char *) malloc(buf_size);
    int len = 0;
    while(true) {
        int read_res = read(d1, buffer ,  buf_size);

        if (read_res < 0) {
            return -1;
        }
        if (read_res == 0) {
            return 0;
        }
        write_str(d2, buffer,  read_res);
        len += read_res;
//        printf("%i\n", read_res);
        if (sbreak == 1) {
            unlink(f2);
            sbreak = 0;
            return 0;
        }
        if (sprint == 1) {
            printf("%i\n", len);
            sprint = 0;
        }
    }
    return 0;
}

