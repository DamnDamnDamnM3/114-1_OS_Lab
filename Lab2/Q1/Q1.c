#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    pid = fork(); // 建立子程序

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } 
    else if (pid == 0) {
        // 子程序執行區
        printf("[%d] Child process running.\n", getpid());
        exit(0);
    } 
    else {
        // 父程序執行區
        wait(NULL); // 等待子程序結束
        printf("[%d] Hello world!\n", getpid());
    }

    return 0;
}
