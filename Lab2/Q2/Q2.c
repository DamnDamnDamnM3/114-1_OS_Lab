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
            // 子程序：執行 programA
            execlp("./programA", "programA", NULL);
            perror("execlp failed"); // execlp 失敗才會執行這行
            exit(1);
        }
    }

    // 父程序：等待所有子程序結束
    while (wait(NULL) > 0);
    printf("[%d] All child processes finished.\n", getpid());

    return 0;
}
