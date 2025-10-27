#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    for (int i = 0; i < 4; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0) {
            // 子行程執行 programA
            if (execlp("./programA", "programA", NULL) == -1) {
                perror("execlp failed");
                exit(1);
            }
        } else {
            // 父行程印出
            printf("I'm parent\n");
        }
    }

    // 父行程等待所有子行程結束
    while (wait(NULL) > 0);
    return 0;
}
