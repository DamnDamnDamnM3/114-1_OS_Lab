#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define SIZE 80

int arr[SIZE][SIZE];
int result[SIZE][SIZE] = {0};
bool isStart = false;

typedef struct my_pid {
    int pid;
} my_pid;

/* 讀取 number.txt 形成 80×80 矩陣 */
void readMatrix()
{
    FILE *fp = fopen("number.txt", "r");
    if (!fp) {
        printf("number.txt not found\n");
        exit(1);
    }
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            fscanf(fp, "%d", &arr[i][j]);
    fclose(fp);
}

/* 矩陣相乘：result = arr × arr */
void multipfy()
{
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++) {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; k++)
                result[i][j] += arr[i][k] * arr[k][j];
        }
}

/* 計算結果矩陣中所有元素總和 */
long sum()
{
    long s = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            s += result[i][j];
    return s;
}

/* 顯示兩個時間點的差值 */
void diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec  = end.tv_sec  - start.tv_sec  - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } else {
        temp.tv_sec  = end.tv_sec  - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    printf("Time = %ld seconds and %ld nanoseconds\n",
           temp.tv_sec, temp.tv_nsec);
}

/* 取得系統最後一個 CPU 核心編號（例如 16 核→回傳 15） */
int getLastCore()
{
    int coreCount = sysconf(_SC_NPROCESSORS_ONLN);
    return coreCount - 1;
}

/* 將子執行緒綁定到最後一個核心  
   不再導向 /dev/null，以便輸出 affinity list */
void setTaskToCore(my_pid *pid)
{
    int core = getLastCore();           // 取得最後一個核心編號
    while (pid->pid == 0)               // 等待子執行緒取得 tid
        ;

    char cmd[128];
    sprintf(cmd, "taskset -cp %d %d", core, pid->pid);
    system(cmd);                        // 執行並讓系統印出 current/new affinity list
}

/* 子執行緒函式 */
void *child(void *arg)
{
    long *ret = calloc(1, sizeof(long));
    my_pid *pid = (my_pid *)arg;

    /* 取得執行緒 ID（tid）並寫回給主程式 */
    pid->pid = syscall(SYS_gettid);

    printf("Process Id: %d\n", getpid());     // 顯示行程 ID（與主程式相同）
    printf("Thread Id: %d\n", pid->pid);      // 顯示執行緒 ID（tid）

    /* 等待主程式通知開始 */
    while (!isStart)
        ;

    multipfy();           // 執行矩陣乘法
    *ret = sum();         // 計算總和
    pthread_exit((void *)ret);
}

int main()
{
    pthread_t thread;
    void *ret;
    my_pid pid = {0};
    struct timespec t1, t2;

    printf("Process Id: %d\n", getpid());      // 主程式 PID
    printf("Thread Id: %ld\n", syscall(SYS_gettid));  // 主程式 TID

    readMatrix();                              // 讀取矩陣資料
    pthread_create(&thread, NULL, child, &pid); // 建立子執行緒

    setTaskToCore(&pid);                       // 綁定到最後一個核心並輸出 affinity list

    sleep(1);                                  // 確保子執行緒準備好
    isStart = true;                            // 通知子執行緒開始運算
    clock_gettime(CLOCK_REALTIME, &t1);        // 開始計時

    pthread_join(thread, &ret);                // 等待子執行緒結束
    clock_gettime(CLOCK_REALTIME, &t2);        // 結束計時

    printf("Sum = %ld\n", *(long *)ret);       // 顯示結果總和
    diff(t1, t2);                              // 顯示耗時

    free(ret);
    return 0;
}
