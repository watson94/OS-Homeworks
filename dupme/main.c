#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int write_str (int begin, int end, char * buffer) {
    int write_res = 0;
    while (begin < end) {
        write_res = write(1, buffer + begin, end - begin);
        if (write_res > 0) {
            begin += write_res;
        }
        if (write_res < 0) {
            perror("Write error");
            exit(2);
        }
    }
    return write_res;
}

template < class T>
void check_null(T buffer) {
    if (buffer == NULL) {
        perror("Allocate error");
        exit(1);
    }
}


void write_twice(int begin, int end, char * buffer) {
    write_str(begin, end, buffer);
    write_str(begin, end, buffer);
}

int main (int argc, char * argv[]) {
    if (argc < 1) {
        return 2;
    }
    int k = atoi(argv[1]);
    int len = 0;
    int lastindex, curindex, i;
    int isWrite = 1;
    char * buffer = (char *) malloc(k + 1);
    check_null(buffer);
    while (1) {
    int r = read(0, buffer + len, k - len + 1);
        if (r == 0) {
            if ((len > 0) && (isWrite == 1) && (len < k + 1)) {
                buffer[len] = '\n'; 
                write_twice(0, len + 1, buffer);
            }
            free(buffer);
            return 0;
        }
        if (r < 0) {
            free(buffer);
            return 1;
        }
        lastindex = 0;
        for (i = 0; i < r; i++) {
            curindex = i + len;
            if (buffer[curindex] == '\n') {
                if (isWrite) {
                    write_twice(lastindex, curindex + 1, buffer);
                } else {
                    isWrite = 1;
                }
                lastindex = curindex + 1;
            } 
        }
        if ((lastindex == 0) &&(curindex == k)) {
            isWrite = 0;
            len = 0;	
        } else {
            memmove(buffer, buffer + lastindex, curindex - lastindex + 1);
            len = curindex - lastindex + 1;
        }
    }
    free(buffer);
}
