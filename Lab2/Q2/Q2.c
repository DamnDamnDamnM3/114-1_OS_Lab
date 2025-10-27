#include <stdio.h>      /* printf */
#include <sys/types.h>  /* pid_t */
#include <unistd.h>     /* fork(), getpid() */
#include <stdlib.h>     /* exit() */
#include <sys/wait.h>   /* wait() */

int main() {
    pid_t pid;

    for (int i = 0; i < 3; i++) {
        pid = fork();                     // 建立子行程
    }

    while (wait(NULL) > 0);               // 父行程等待所有子行程結束

    if (pid > 0) {                        // 父行程
        printf("I'm parent\n");
    }
    else if (pid == 0) {                  // 子行程
        if (execlp("./programA", "programA", NULL) == -1) {
            perror("execlp failed");
            exit(1);
        }
    }
    else {
        perror("fork failed");
        exit(1);
    }

    return 0;
}
