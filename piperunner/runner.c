#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector.h>
void  write_str (int  fd, char *str, int size) {
    int i = 0;
    while (i < size) {
        int write_res = write(fd, str + i, size - i);
        if (  write_res > 0) {
            i += write_res;
        }
    }
}



int findseparator(char separator, char * str, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (str[i] == separator) {
            return i;
        }
    }
    return -1;
}
int main (int argc , char ** argv) {
    const int maxsize = 4096;
    char * buffer = (char *) malloc(maxsize );
    char * filename = argv[1];

    int  fdin = open(filename, O_RDWR, S_IRWXU);
    int  output = 1;

    char separator = "\0";
    int curlen = 0;
    int myeof = 0;
    
    int curind = 0;
    int curword = 0;
    int lastsep = 0;
    std::vector<std::vector<std::vector<char> > > commands;
    while (!myeof) {
        if (curlen == maxsize) {
            return 3;
        }

        int read_res = read(fdin , buffer, maxsize);
        if (read_res < 0) {
           return -1;
        }
        int from = 0;

        printf ("mhere\n");
        int pos = 0; 

        for ( int i = 0; i < maxsize; i++) {
            if ( buffer[i] == '\0' ) {
                if (issep == 0) {
                    curword++;
                    std::vector<char> tmp1;
                    commands[curind].push_bach(tmp1);
                } else {
                    curind++;
                    std::vector<std::vector<char > > tmp;
                    commands[curind].push_back(tmp);
                    std::vector<char> tmp1;
                    commands[curind].push_bach(tmp1);
                    curword = 0;
                }
            } else {
                commands[curind][curword].push_back(buffer[i]);
            }
        }
    }
    int i, j, k;
    for (i = 0; i < commands.size() - 1; i++) {
        for (j = 0; j < commands[i].size() - 1; j++) {
            for (k = 0; k < commands[i][j].size - 1; k++) {
                printf("%c\n", commands[i][j][k]);
            }
            printf(" ");
        }
        printf("\n");
    }
}
