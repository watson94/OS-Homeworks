#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
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


/*
int findseparator(char separator, char * str, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (str[i] == separator) {
            return i;
        }
    }
    return -1;
} */
int min (int a, int b) {
    if (a < b) return a;
    return b;
}
int main (int argc , char ** argv) {
    const int maxsize = 4096;
    char * buffer = (char *) malloc(maxsize);
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


/*    char separator = "\n";
    int curlen = 0;
    int myeof = 0;

    while (!myeof) {
        if (curlen == maxsize) {
            return 3;
        }

        int read_res = read(filelist, buffer, maxsize - curlen + 1);
        if (read_res < 0) {
           return -1;
        }
        int from = curlen;
        curlen += read_res;
        if (read_res == 0) {
            myeof = 1;
            if (curlen == maxsize) {
                return 3;
            } else {
                if (curlen > 0) {
                    buffer[curlen] = '\n';
                    write_str(output, buffer, curlen + 1);
                }
            }
            break;
        }
        printf ("mhere\n");
        int pos = 0; 
        while ((pos = findseparator(separator, buffer + from, curlen - from)) >= 0 ) {
            write_str(output, buffer, pos + 1);
            memmove(buffer, buffer + pos + 1, curlen - pos);
            curlen -= (pos + 1); 
        }
    }*/
}
