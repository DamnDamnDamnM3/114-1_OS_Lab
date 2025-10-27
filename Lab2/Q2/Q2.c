#include <stdio.h> /* printf */
#include <sys/types.h> /* pid_t */
#include <unistd.h> /* fork(), getpid() */
#include <stdlib.h> /* exit() */
#include <sys/wait.h> /* wait() */
//execlp(const char *file, const char *arg, ..., NULL);
//execlp(尋找程式路徑, 傳遞給程式的第一個引數(通常為程式名稱), ..., NULL);
//該程式可利用argv[0]取得程式名稱

int main() {
    pid_t pid;

    /* 建立三個 child（父進程每次 fork，子 process 跳出迴圈） */
    for(int i = 0; i < 3; i++)
    {
        pid = fork();
        if (pid == 0) {
            /* 子程序跳出迴圈，避免繼續 fork */
            break;
        }
        /* 父程序則繼續下一次迴圈以產生更多 child */
    }

    /* 父進程等待所有子程序結束 */
    while(wait(NULL) > 0);

    if(pid > 0)
    {
        /* 只在父程序印出訊息（可對照 lab2-1 範例） */
        printf("[%d] Hello world!\n", getpid());
    }
    else if(pid == 0)
    {
        /* 在子程序中用 execlp 跳到 programA 執行 */
        if (execlp("./programA", "programA", NULL) == -1)  /*利用execlp使process跳到別的程式執行*/
        {                               /*若execlp執行失敗，回傳-1*/
            perror("execlp failed"); /*印出錯誤資訊*/
            exit(1);   
        }
    }
    else
    {
        perror("fork failed");
        exit(1);
    }
    return 0;
}
