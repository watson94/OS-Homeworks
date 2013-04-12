#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * buffer;
char ** myargv;
int argpos;

void write_str(char * buf, int size) {
        int begin = 0;
        while(begin < size) {
            int w = write(1, buf + begin, size - begin);
            begin += w;
        }
}


int finddelim (char delim, char * buf, int from, int len) {
    int i = 0;
    for (i = from; i < from + len; i++) {
        if (buf[i] == delim) {
            return i;
        }
    }
    return -1;
}



void run_cmd_on(char * buf, int len){
    printf("im here\n");
    char * myargv[5];
    myargv[1] = "ls";
    myargv[2] = "-la";
    myargv[3] = 0;
    int parent = fork();
    if( parent == 0 ) {
        printf("im child\n"); 
        execvp(myargv[1], &myargv[1]); 
        exit(1);
    }
    else {
        int status;
        waitpid(parent, &status, 0);
        printf("im parent\n"); 
        if(WIFEXITED(status) && (WEXITSTATUS( status ) == 0 )) {
            buf[len] = '\n';
            write_str(buf, len + 1);
        }
    } 
}



int main(int argc, char * argv[]) {
    char delim = '\n';
    int bufsize = 4096;
    int res;

    while(res = getopt(argc, argv, "nzb:") != -1){
            switch(res) {
                case 'n' :
                     break;
                case 'z':
                     delim = 0;
                     break;
                case 'b' :
                     bufsize = optarg;        
                     break;
                case '?' : 
                    return 1;
                    break;  
                default : 
                    return 2;
            }
    }

    int len = 0;
    int my_eof = 0;
    buffer = malloc(bufsize + 1); 
    while (!my_eof) {
        if (len == bufsize) {
            return 178;
        }
        int r = read(0, buffer + len, bufsize - len + 1);
        if (r == 0) {
            my_eof = 1;
            buffer[len] = delim;
        }
        if (r < 0) {
            return 1;
        }
        int from = len;
        len += r;
        int delimpos;
        while((delimpos = finddelim(delim, buffer, from, len - from)) >= 0) {
            run_cmd_on(buffer, delimpos);
            memmove(buffer, buffer+delimpos + 1, len - delimpos - 1);
            from = 0;
            len -= delimpos + 1;
        }        
    }
    if (len > 0) {
        if (len + 1 > bufsize) {
            return 128;
        }
        buffer[len + 1] = delim;
        run_cmd_on(buffer, len + 1);
    }
}
