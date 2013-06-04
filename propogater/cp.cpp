#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <wait.h>

char * getstring(int x) {
    char * tmp = (char * ) malloc(100);
    int k = 0;
    while(x > 0) {
        tmp[k] = '0' + (x % 10);
        x = x/10;
        k++;
    }
    char * ans = (char *) malloc( k + 1);
    for (int i = 0; i < k; i++) {
        ans[i] = tmp[k - i - 1];
    }
    ans[k] = 0;

    free(tmp);

    return ans;
}

int main(int argc, char ** argv) {
    if (argc < 3) {
        printf("There are should be at least 2 arguments\n");
        return 1;
    }   
    if (argc % 2 != 1) {
        printf("Number of arguments must be even\n");
        return 2;
    }
    char ** myargv = (char **) malloc(argc * sizeof (char *) );
    for (int i = 1; i < argc; i++) {
        int x = open(argv[i], O_RDWR, S_IRWXU);
        myargv[i] = getstring(x);
    }
    myargv[argc] = 0;
    myargv[0] = (char *) "./propogater\0";
    int pid = fork();
    if (pid == 0) {
        execvp(myargv[0], myargv);
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
//    printf("I'm here");
    }
    return 0;
}
