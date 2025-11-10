#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>
#define SIZE 80       // 矩陣大小（80x80）
#define THREAD 4      // 執行緒數量（分成4份處理）

int arr[SIZE][SIZE];               // 讀取的原始矩陣資料
int result[SIZE][SIZE] = {0};      // 存放乘法結果
bool isStart = false;              // 控制執行緒何時開始運算

/* 不可修改：讀入矩陣資料 */
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
            fscanf(fp, "%d", &arr[i][j]);   // 將數字逐一讀入二維陣列
        }
    }
    fclose(fp);
}

/* 不可修改：計算所有元素的加總（用來驗證結果） */
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

/* 不可修改：計算兩個時間點的差值，用來顯示執行時間 */
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

/* 填空部分：執行矩陣乘法的函式
   startIndex 與 endIndex 表示該執行緒要負責的列範圍。
   例如：thread0 處理第 0~19 列，thread1 處理第 20~39 列，以此類推。
   每個執行緒都呼叫此函式，分別運算自己負責的部分。 */
void multipfy(int startIndex, int endIndex)
{
    for (int i = startIndex; i < endIndex; i++)     // 控制行的範圍
    {
        for (int j = 0; j < SIZE; j++)              // 逐一計算每一格的結果
        {
            result[i][j] = 0;
            for (int k = 0; k < SIZE; k++)          // 進行矩陣乘法運算
            {
                result[i][j] += arr[i][k] * arr[k][j];
            }
        }
    }
}

/* 子執行緒函式：接收一段範圍 [startIndex, endIndex)
   每個執行緒會：
   1. 顯示自己的 PID / TID（方便觀察）
   2. 等待主執行緒設定 isStart = true
   3. 呼叫 multipfy() 運算屬於自己的矩陣部分
   4. 結束執行緒 */
void *child(void *arg)
{
    int *range = (int *)arg;
    int startIndex = range[0];
    int endIndex = range[1];

    // 取得當前行程與執行緒ID
    printf("Child pid = %d\n", getpid());
    printf("Child tid = %ld\n",
           syscall(SYS_gettid), startIndex, endIndex);

    // 等待主程式發出開始訊號
    while (!isStart)
        ;

    // 執行自己負責區段的矩陣運算
    multipfy(startIndex, endIndex);

    // 執行完畢後離開執行緒
    pthread_exit(NULL);
}

/* 主程式 main()
   功能：
   1. 建立 THREAD 數量的子執行緒
   2. 將矩陣平均分成 THREAD 份
   3. 在所有執行緒開始運算後計時
   4. 等待所有執行緒結束並顯示結果 */
int main()
{
    pthread_t *threads;                  // 儲存所有子執行緒的 ID
    struct timespec timeStart, timeEnd;  // 紀錄執行時間

    printf("Main pid = %d\n", getpid());
    printf("Main tid = %ld\n", syscall(SYS_gettid));

    readMatrix();  // 讀取矩陣資料

    threads = (pthread_t *)malloc(THREAD * sizeof(pthread_t));
    int ranges[THREAD][2];               // 每個執行緒的工作範圍
    int per = SIZE / THREAD;             // 每個執行緒分配的列數（此例每個20列）

    /* 建立子執行緒並分配運算區段 */
    for (int i = 0; i < THREAD; i++)
    {
        ranges[i][0] = i * per;                      // 起始列
        ranges[i][1] = (i == THREAD - 1) ? SIZE : (i + 1) * per;  // 結束列
        pthread_create(&threads[i], NULL, child, ranges[i]);
    }

    sleep(1);                      // 等待所有執行緒建立完成
    isStart = true;                // 通知所有執行緒開始運算
    clock_gettime(CLOCK_REALTIME, &timeStart);  // 開始計時

    /* 等待所有執行緒完成 */
    for (int i = 0; i < THREAD; i++)
    {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_REALTIME, &timeEnd);   // 結束計時

    /* 顯示最終結果 */
    printf("Sum = %ld\n", sum());    // 檢查結果正確性（所有元素總和）
    diff(timeStart, timeEnd);        // 顯示運算時間

    free(threads);                   // 釋放記憶體
    return 0;
}
