#include <stdio.h>      // printf()
#include <sys/types.h>  // pid_t
#include <unistd.h>     // fork(), getpid()
#include <sys/wait.h>   // wait()

int main() {
    pid_t pid;

    // 建立多個 child process
    for (int i = 0; i < 7; i++) {
        pid = fork();  // 建立子行程
        if (pid < 0) {
            perror("fork failed");
            return 1;
        } else if (pid == 0) {
            // 子行程印出自己的 PID
            printf("[%d] Hello world!\n", getpid());
            return 0; // 子行程結束，不再進入下一次迴圈
        }
        // 父行程繼續下一次迴圈
    }

    // 父行程等待所有子行程結束
    while (wait(NULL) > 0);

    // 父行程結束前印一次
    printf("[%d] Hello world!\n", getpid());
    return 0;
}
