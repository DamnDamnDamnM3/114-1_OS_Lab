#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define SIZE 80

int arr[SIZE][SIZE];               // 儲存讀入的矩陣資料
int result[SIZE][SIZE] = {0};      // 儲存矩陣相乘結果
bool isStart = false;              // 控制子執行緒開始運算的旗標

// 結構：用來傳遞子執行緒的 thread ID (TID)
typedef struct my_pid
{
    int pid;
} my_pid;

/* 不可修改：讀取矩陣資料 */
void readMatrix()
{
    FILE *fp;
    fp = fopen("number.txt", "r");
    if (!fp)
    {
        printf("number.txt not found\n");
        exit(1);
    }
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            fscanf(fp, "%d", &arr[i][j]);
        }
    }
    fclose(fp);
}

/* 不可修改：執行矩陣乘法 arr × arr */
void multipfy()
{
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            for (int k = 0; k < SIZE; k++)
            {
                result[i][j] += arr[i][k] * arr[k][j];
            }
        }
    }
}

/* 不可修改：計算所有元素總和 */
long sum()
{
    long ret = 0;
    for (int i = 0; i < SIZE; i++)
    {
        for (int j = 0; j < SIZE; j++)
        {
            ret += result[i][j];
        }
    }
    return ret;
}

/* 不可修改：計算時間差 */
void diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0)
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    printf("Time = %ld seconds and %ld nanoseconds\n", temp.tv_sec, temp.tv_nsec);
}

/* 填空 1：取得最後一個 CPU 核心的編號
   sysconf(_SC_NPROCESSORS_ONLN) 可取得系統核心數。
   例如有 4 核心則回傳 4，因此最後一個核心編號為 3。 */
int getLastCore()
{
    int coreCount = sysconf(_SC_NPROCESSORS_ONLN);
    return coreCount - 1;
}

/* 填空 2：將子執行緒綁定到指定核心
   使用 system() 執行 Linux 指令 "taskset -cp core pid"。
   因為子執行緒在建立時會花一點時間取得 tid，
   所以這裡使用 while(pid->pid == 0) 等待子執行緒將自己的 tid 寫入結構。 */
void setTaskToCore(my_pid *pid)
{
    int core = getLastCore();      // 取得最後一個核心編號
    while (pid->pid == 0)
        ;                          // 等待子執行緒取得自己的 tid

    // 建立指令字串並執行 taskset 綁定核心
    char cmd[128];
    sprintf(cmd, "taskset -cp %d %d > /dev/null", core, pid->pid);
    system(cmd);
}

/* 填空 3：子執行緒的主體函式
   子執行緒負責：
   1. 取得自己的 thread ID (tid)
   2. 等待主程式設定 isStart 為 true
   3. 執行矩陣乘法與加總
   4. 回傳結果 */
void *child(void *arg)
{
    long *ret = calloc(1, sizeof(long));  // 用來儲存回傳值
    my_pid *pid = (my_pid *)arg;

    // 取得 thread ID (tid) 並寫入 pid 結構給主程式
    pid->pid = syscall(SYS_gettid);

    // 顯示子執行緒的 PID 與 TID
    printf("Child: pid = %d\n", getpid());
    printf("Child: tid = %d\n", pid->pid);

    /* 不可修改的部分：等待主程式開始運算 */
    while (!isStart)
        ;
    multipfy();
    *ret = sum();
    /* ------------ */

    // 結束執行緒並回傳結果
    pthread_exit((void *)ret);
}

int main()
{
    pthread_t thread;              // 子執行緒 ID
    void *ret;                     // 用來接收子執行緒的回傳值
    my_pid pid = {0};              // 用來儲存子執行緒 tid
    struct timespec timeStart, timeEnd;

    // 顯示主執行緒的 PID 與 TID
    printf("Main: pid = %d\n", getpid());
    printf("Main: tid = %ld\n", syscall(SYS_gettid));

    readMatrix();                  // 讀取 number.txt 的矩陣資料

    // 建立子執行緒，傳入 pid 結構的位址
    pthread_create(&thread, NULL, child, &pid);

    // 填空 4：將子執行緒綁定到最後一個核心
    setTaskToCore(&pid);

    sleep(1);                      // 等待子執行緒初始化完成

    isStart = true;                // 通知子執行緒開始運算
    clock_gettime(CLOCK_REALTIME, &timeStart);

    // 等待子執行緒完成
    pthread_join(thread, &ret);
    clock_gettime(CLOCK_REALTIME, &timeEnd);

    // 顯示最終運算結果與花費時間
    printf("Result = %ld\n", *(long *)ret);
    diff(timeStart, timeEnd);

    free(ret);                     // 釋放動態配置記憶體
    return 0;
}
