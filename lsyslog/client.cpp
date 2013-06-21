#include <stdio.h>
#include "lsyslog.cpp"
int main() {
    openlog("ident", 2, 3);
    for (int i = 0; i < 1000000; i++) {
        syslog(2, "qwqwqw");
        printf("%i\n", i);

    }
    printf("Im wrote\n");
    closelog();
}
