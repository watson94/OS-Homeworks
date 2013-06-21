
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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

int min (int a, int b) {
    if (a < b) return a;
    return b;
}

int main (int argc , char ** argv) {
    
    if (argc < 1) {
        return 2;
    }
    const int maxsize = 4096;
    const int MIL = 1000000;
    const int THO = 1000;
    char * buffer = (char *) malloc(maxsize);

    if (buffer == NULL) {
        return 1;
    }
    int in = 0;
    int out = 1;
    int n = atoi(argv[1]); // count of words per second
    int curlen = 0;
    int r;
    while ((r = read(in, buffer + curlen, maxsize - curlen)) > 0) {
        curlen += r;
        int numword = 0;
        int bufpointer = 0;
        printf("Imhere\n");        
        struct timespec start ;
        int rr = clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        if (rr == -1) {
                printf("qwqw\n");
                return 2;
        }
        for (int j = 0; j < curlen; j++) {
            if (buffer[j] == '\n') {
                write_str(out, buffer + bufpointer, j - bufpointer + 1);
                bufpointer = j + 1;
                numword++;
                if (numword == n) {
                    break;
                }
            }
        }
        if (numword == 0) {
            write_str(out, buffer, curlen);
        }
        if (numword == n) {
            struct timespec  finish ;
            int rr = clock_gettime(CLOCK_MONOTONIC_RAW, &finish);
             if (rr == -1) {
                return 2;
             }
             
            printf("imhere2\n");
            long long stime =  (start.tv_sec * MIL + start.tv_nsec);
            long long ftime = (finish.tv_sec * MIL + finish.tv_nsec);
            long long sleeptime = (MIL * THO - (ftime - stime)) / THO;
            usleep(sleeptime);
        }
        memmove(buffer, buffer + bufpointer, curlen - bufpointer);
        curlen -= bufpointer;
    }
    if (r == -1) {
        return 3;
    }
    return 0;
}
