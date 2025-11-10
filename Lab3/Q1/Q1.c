/*
 * 取得最後一個可用 CPU core 的編號
 * sysconf(_SC_NPROCESSORS_ONLN) 回傳線程可用的 core 數量（int）
 * 因為 core 編號從 0 開始，最後一個 core 的編號為 count - 1
 */
int getLastCore()
{
    int coreCount = sysconf(_SC_NPROCESSORS_ONLN);
    return coreCount - 1;
}

/*
 * 將指定 thread（以其 tid 存在於 pid->pid）綁定到系統的最後一個 core
 * 注意：此函式使用忙等等待 pid->pid 被 child thread 設定。
 */
void setTaskToCore(my_pid *pid) /*將Thread綁定到指定core*/
{
    int core = getLastCore(); /* 獲得最後一個 core 編號（0-based） */
    while (pid->pid == 0)
        ; /* 等待 child thread 設定其 TID 到 pid->pid（忙等） */
    /* 使用外部指令 taskset 綁定該 tid 到指定 core，輸出導到 /dev/null 隱藏訊息 */
    char cmd[128];
    sprintf(cmd, "taskset -cp %d %d > /dev/null", core, pid->pid);
    system(cmd);
}

/*
 * 子執行緒函式
 * 1) 以 syscall 取得 thread id（tid），並存入傳入的 my_pid 結構以供主執行緒使用
 * 2) 印出 process id 與 thread id（便於除錯與驗證）
 * 3) 等待主執行緒設定全域啟動旗標 isStart
 * 4) 執行 multipfy()（可能為矩陣乘法等準備步驟），再以 sum() 取得結果
 * 5) 透過 pthread_exit 回傳結果指標
 */
void *child(void *arg) /*子執行緒*/
{
    long *ret = calloc(1, sizeof(long));
    my_pid *pid = (my_pid *)arg;
    /* 取得 thread id (tid)，並存入呼叫端提供的結構 */
    pid->pid = syscall(SYS_gettid);

    /* 印出 PID 與 TID（方便觀察不同執行緒資訊） */
    printf("Child: pid = %d\n", getpid());
    printf("Child: tid = %d\n", pid->pid);

    /* 不要變更以下程式流程：等待啟動訊號並執行計算 */
    while (!isStart)
        ;
    multipfy();
    *ret = sum();

    /* 使用 pthread_exit 回傳動態配置的結果 */
    pthread_exit((void *)ret);
}

/*
 * 程式主流程 (main)
 * - 印出主程序 PID/TID
 * - 讀入矩陣或初始化資料 (readMatrix)
 * - 建立子執行緒，並將該執行緒綁定到指定 core
 * - 等待短暫時間確保綁定完成，設定 isStart 啟動子執行緒的計算
 * - 計時、等待子執行緒結束並取得結果，最後印出並計算耗時
 */
int main()
{
    pthread_t thread;
    void *ret;
    my_pid pid = {0};
    struct timespec timeStart, timeEnd;

    /* 印出主程序的 PID 與 TID（便於比對 child） */
    printf("Main: pid = %d\n", getpid());
    printf("Main: tid = %ld\n", syscall(SYS_gettid));

    /* 讀取或初始化矩陣資料（實作於其他檔案） */
    readMatrix();

    /* 建立子執行緒，child 會將自身 tid 寫入 pid 結構 */
    pthread_create(&thread, NULL, child, &pid);

    /* 將 child thread 綁定到最後一個 core（由 child 設定 pid->pid 後執行） */
    setTaskToCore(&pid);

    /* 等待短暫時間以確保綁定與初始動作完成 */
    sleep(1);

    /* 讓 child 開始真正的計算並記錄起始時間 */
    isStart = true;
    clock_gettime(CLOCK_REALTIME, &timeStart);

    /* 等待 child 結束並取得回傳的結果指標 */
    pthread_join(thread, &ret);

    /* 記錄結束時間並印出結果 */
    clock_gettime(CLOCK_REALTIME, &timeEnd);
    printf("Result = %ld\n", *(long *)ret);

    /* 計算並顯示耗時（實作於其他檔案） */
    diff(timeStart, timeEnd); // time spent

    return 0;
}
