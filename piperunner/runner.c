#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <stdlib.h>
#include <sys/wait.h>
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

char ** parsecommand(std::vector<std::vector<char > > com) {
    char ** ans = (char **)malloc(com.size() + 1);
    for (int i = 0; i < com.size(); i++) {
        ans[i] = (char *) malloc(com[i].size() + 1);
        for (int j = 0; j < com[i].size(); j++) {
            ans[i][j] = com[i][j];
        }
        ans[i][com[i].size()] = 0;
    }
    ans[com.size()] = NULL;
    return ans;
}



int main (int argc , char ** argv) {
    const int maxsize = 4096;
    char * buffer = (char *) malloc(maxsize );
    char * filename = argv[1];

    int  fdin = open(filename, O_RDWR, S_IRWXU);
    int  output = 1;

    char separator = '\0';
    int curlen = 0;
    int myeof = 0;
    
    int curword = 0;
    int issep = 0;
    int iscreate = 1;
    std::vector<std::vector<std::vector<char> > > commands;
    int curind = -1;
    while (1) {
        if (curlen == maxsize) {
            return 3;
        }

        int read_res = read(fdin , buffer, maxsize);
        if (read_res < 0) {
           return -1;
        }
        printf("imhere\n");
        if (read_res == 0) {
            break;
        }
        int from = 0;

        int pos = 0; 

        for ( int i = 0; i < read_res; i++) {
            if (iscreate == 1) {
                curind++;
                curword = 0;
                std::vector<std::vector<char > > tmp;
                commands.push_back(tmp);
                std::vector<char> tmp1;
                commands[curind].push_back(tmp1);
            }
            if ( buffer[i] == '\0' ) {
                if (issep == 0) {
                    curword++;
                    std::vector<char> tmp1;
                    commands[curind].push_back(tmp1);
                    iscreate = 0;
                    issep = 1;
                } else {
                    iscreate = 1;
                    issep = 0;
                }
            } else {
                commands[curind][curword].push_back(buffer[i]);
                iscreate = 0;
                issep = 0;
            }
        }
    }
    

    int curpipe[2];
    int lastpipe[2];
    pipe(lastpipe);
    pipe(curpipe);
    dup2(0, lastpipe[0]);
//    for (int i = 0; i < commands.size() - 1; i++) {
        int pid = fork();
        if (pid == 0) {
            dup2(lastpipe[0], 0);
//            dup2(curpipe[1], 1);
            char ** mycommand = parsecommand(commands[0]);
            printf("%s\n", mycommand[0]);
            printf("%s\n", mycommand[1]);
            printf("%s\n", mycommand[2]);
            execvp(mycommand[0], mycommand);
        } else {
            waitpid(pid,NULL, 0);
            lastpipe[0] = curpipe[0];
            lastpipe[1] = curpipe[1]; 
            pipe(curpipe);
        }
//    }







/*    int i, j, k;
    for (i = 0; i < commands.size() ; i++) {
        for (j = 0; j < commands[i].size() - 1; j++) {
            for (k = 0; k < commands[i][j].size() ; k++) {
                printf("%c\n", commands[i][j][k]);
            }
            printf("new word\n");
        }
        printf("new command\n");
    }*/ 
}
