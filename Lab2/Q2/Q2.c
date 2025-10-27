#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // 建立三個子程序
    for (int i = 0; i < 3; i++) {
        pid = fork();

        if (pid < 0) {
            perror("fork failed");
            exit(1);
        } 
        else if (pid == 0) {
            // 子程序執行 programA
            execlp("./programA", "programA", NULL);
            perror("execlp failed");
            exit(1);
        }
        // 父程序不做任何事，直接繼續下一個迴圈
    }

    // 父程序等待所有子程序結束
    while (wait(NULL) > 0);

    printf("I'm parent\n");

    return 0;
}
