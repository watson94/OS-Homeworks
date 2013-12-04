#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <map>
#include <iostream>

int write_str (size_t begin, size_t end, const char * buffer) {
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

template <class T>
void check_null(T* buffer) {
    if (buffer == NULL) {
        perror("Allocate error");
        exit(1);
    }
}


void write_twice(int begin, int end, char * buffer) {
    write_str(begin, end, buffer);
    write_str(begin, end, buffer);
}



char * buffer;
int k;
int len;
bool isWrite;

std::pair<bool , std::string> readLine () {
    std::pair<int, std::string> ans;
    while (1) {
        int posn = -1;
        for (int i = 0; i < len; i++) {
            if (buffer[i] == '\n') {
                posn = i;
                break;
            }
        }
        
        if ( posn == -1) {
            if (len == k) {
                len = 0;
                isWrite = false;
            }
            int r = read(1, buffer + len, k - len);
            if (r == 0) {
                if ((isWrite) && (len > 0)) {
                    std::string ansb(buffer, len);
                    len = 0;
                    ans.second = ansb;
                    ans.first = false;
                } else {
                    ans.first = true;
                }
                return ans;
            }
            if (r < 0) {
                free(buffer);
                exit(3);
            }
            len += r;
        } else {
            if (isWrite) {
                ans.first = false;
                std::string ansb(buffer, posn + 1);
                ans.second = ansb;
                memmove(buffer, buffer + posn + 1, len - posn - 1); 
                isWrite = true;
                len -= posn + 1;
                return ans;
            } else {
                memmove(buffer, buffer + posn + 1, len - posn - 1); 
                isWrite = true;
                len -= posn + 1;
            }
        }
    }
}

void writeLine(std::string str) {
    write_str(0, str.size(),  str.data());
}


int main (int argc, char * argv[]) {
    if (argc < 1) {
        return 2;
    }
    k = atoi(argv[1]);
    isWrite = true;
    len = 0;
    buffer = (char *) malloc(k + 1);
    check_null(buffer);
    while (1) {
        std::pair<bool, std::string> r = readLine();
        if (r.first) break;
        writeLine(r.second);
        writeLine(r.second);
    }
}
