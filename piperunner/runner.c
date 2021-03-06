#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <stdlib.h>
#include <sys/wait.h>

char ** parsecommand(std::vector<std::vector<char > > com) {
    char ** ans = (char **) malloc((com.size() + 1) * sizeof(char*));
    for  (size_t i = 0; i < com.size(); i++) {
        ans[i] = (char *) malloc(com[i].size() + 1);
        for (size_t j = 0; j < com[i].size(); j++) {
            ans[i][j] = com[i][j];
        }
        ans[i][com[i].size()] = 0;
    }
    ans[com.size()] = 0;
    return ans;
}

void myfree(char ** arg, size_t size) {
    for (size_t i = 0; i < size; i++) {
        free(arg[i]);
    }
    free(arg);
}

int main (int argc , char ** argv) {
    if (argc < 1) {
        return 1;
    }

    const int maxsize = 4096;

    char * buffer = (char *) malloc(maxsize );

    if (buffer == NULL) {
        return 2;
    }

    char * filename = argv[1];

    int  fdin = open(filename, O_RDWR, S_IRWXU);

    int curlen = 0;
    
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
        if (read_res == 0) {
            break;
        }

        for (int i = 0; i < read_res; i++) {
            if (iscreate == 1) {
                if (curind >= 0) {
                    commands[curind].pop_back();
                }
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
    
    commands.pop_back();

    int curpipe[2];
    int lastpipe[2];
    pipe(lastpipe);
    pipe(curpipe);
    dup2(0, lastpipe[0]);
    for (size_t i = 0; i < commands.size() ; i++) {
        int pid = fork();
        char ** mycommand = parsecommand(commands[i]);
        if (pid == 0) {
            dup2(lastpipe[0], 0);
            close(lastpipe[0]);
            close(lastpipe[1]);
            if (i != commands.size() - 1) { 
                dup2(curpipe[1], 1);

            } 
                
            close(curpipe[0]);
            close(curpipe[1]);
            execvp(mycommand[0], mycommand);
            exit(1);
        } else {
            if ( i == commands.size() - 1 ) {
                waitpid(pid, NULL, 0);
            }
            close(curpipe[1]);
            lastpipe[0] = curpipe[0];
            lastpipe[1] = curpipe[1]; 
            pipe(curpipe);
            myfree(mycommand, commands[i].size());
        }
    }
}
