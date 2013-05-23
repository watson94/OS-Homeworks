#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
void  write_str (int  fd, char *str, int size) {
    int i = 0;
    while (i < size) {
        int write_res = write(fd, str + i, size - i);
        if (  write_res > 0) {
            i += write_res;
        }
    }
}

int min (int a, int b) {
    if (a < b) return a;
    return b;
}

int main (int argc , char ** argv) {
    
    if (argc < 2) {
        return 2;
    }
    const int maxsize = 4096;
    char * buffer = (char *) malloc(maxsize);

    if (buffer == NULL) {
        return 1;
    }

    char * src = argv[1];
    char * dst = argv[2];
    int bytes = atoi(argv[3]); // byte/seconds
    int in = open(src, O_RDONLY, 0);
    int out = open(dst, O_CREAT | O_RDWR, S_IRWXU);
    int n;
    int curoutbytes = 0;
    while ((n = read(in, buffer, min(bytes - curoutbytes, maxsize))) > 0) {
        curoutbytes += n;
        if (curoutbytes == bytes) {
            sleep(1);
            curoutbytes = 0;
        }
        write_str(out, buffer, n);
    }

    return 0;
}
