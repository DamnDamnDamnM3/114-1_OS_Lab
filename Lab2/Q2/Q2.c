#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    for (int i = 0; i < 3; i++) {
        pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } 
        else if (pid == 0) {
            execlp("./programA", "programA", NULL);
            perror("execlp failed");
            exit(1);
        }
        // 父行程不做事，繼續下一圈
    }

    // 父行程等待所有子行程結束
    while (wait(NULL) > 0);
    printf("I'm parent\n");

    return 0;
}
