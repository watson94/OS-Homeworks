#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char ** argv) {
    if (argc < 1) {
        return 2;
    }

    const char * newFile = "newFile.tmp";
    const char * oldFile = "oldFile.tmp";

    int timeout = atoi(argv[1]);
//  printf("imhere\n");   


    while (1)  {
        int newfile = open(newFile, O_CREAT  | O_RDWR, S_IRWXU);
        int pidexec = fork();
        if (pidexec == 0) {
            dup2(newfile, 1);
            execvp(argv[2], &argv[2]);
        } else { 
            waitpid(pidexec, 0, 0);
        }
        int pidcat = fork();
        if (pidcat == 0) {
           execlp("cat", "cat", newFile, NULL);
        } else {
            waitpid(pidcat, 0, 0);
        }
        int piddiff = fork();
        if (piddiff == 0) {
           execlp("diff", "diff", "-u", newFile, oldFile, NULL);
        } else {
           waitpid(piddiff, 0, 0);
        }
        int pidmv = fork();
        if (pidmv == 0) {
           execlp("mv", "mv", newFile, oldFile, NULL);
        } else {
           waitpid(pidmv, 0, 0);
        }
        sleep(timeout); 
    }
    return 0;
}
