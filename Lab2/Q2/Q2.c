#include <stdio.h>      // printf
#include <sys/types.h>  // pid_t
#include <unistd.h>     // fork(), getpid(), execlp()
#include <stdlib.h>     // exit()
#include <sys/wait.h>   // wait()

int main() {
    pid_t pid;

    /* 建立4個 child process */
    for (int i = 0; i < 4; i++) {
        pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } 
        else if (pid == 0) {
            if (execlp("./programA", "programA", NULL) == -1) {
                perror("execlp failed");
                exit(1);
            }
        }
        // 父行程繼續迴圈建立下一個子行程
    }

    // 父行程等待所有子行程結束
    while (wait(NULL) > 0);

    printf("Parent [%d] all children finished.\n", getpid());
    return 0;
}
