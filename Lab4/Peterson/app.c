/*
 * Peterson 演算法實作
 * 這是一個用於兩個執行緒的互斥鎖演算法，不需要硬體支援的特殊指令
 * 透過 flag 和 turn 兩個變數來確保同一時間只有一個執行緒能進入臨界區
 */

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

// 共享的資料，兩個執行緒會同時存取
int data = 0;

// turn: 表示目前輪到哪個執行緒進入臨界區 (0 或 1)
atomic_int turn = 0;

// flag[2]: 表示執行緒是否想要進入臨界區
// flag[i] = 1 表示執行緒 i 想要進入臨界區
// flag[i] = 0 表示執行緒 i 不想進入臨界區
atomic_int flag[2] = {0, 0};

// 每個執行緒執行的迴圈次數
#define LOOP 100000

/*
 * Peterson 演算法的 lock 函數
 * id: 執行緒的識別碼 (0 或 1)
 * 
 * 演算法步驟：
 * 1. 設定 flag[id] = 1，表示我想要進入臨界區
 * 2. 設定 turn = other，讓對方優先（禮讓）
 * 3. 等待直到：對方不想進入（flag[other] = 0）或輪到我（turn = id）
 */
void lock(int id) {
    int other = 1 - id;  // 計算另一個執行緒的 ID
    
    // 步驟 1: 宣告我想要進入臨界區
    atomic_store(&flag[id], 1);
    
    // 步驟 2: 禮讓對方，讓對方優先進入
    atomic_store(&turn, other);
    
    // 步驟 3: 等待條件滿足
    // 如果對方也想進入（flag[other] = 1）且現在輪到對方（turn = other），
    // 則持續等待（busy wait），直到條件不成立
    while (atomic_load(&flag[other]) && atomic_load(&turn) == other) {
        // busy wait: 持續檢查條件，直到可以進入臨界區
    }
}

/*
 * Peterson 演算法的 unlock 函數
 * id: 執行緒的識別碼 (0 或 1)
 * 
 * 離開臨界區時，將 flag[id] 設為 0，表示不再需要進入臨界區
 */
void unlock(int id) {
    atomic_store(&flag[id], 0);
}

/*
 * 執行緒函數：執行減法操作
 * 這個執行緒會執行 LOOP 次 data-- 操作
 */
void *subchild(void *arg) {
    int id = 1;  // 這個執行緒的 ID 是 1
    
    for (int i = 0; i < LOOP; i++) {
        lock(id);      // 取得鎖，進入臨界區
        data--;        // 臨界區操作：減少 data
        unlock(id);    // 釋放鎖，離開臨界區
    }
    return NULL;
}

/*
 * 執行緒函數：執行加法操作
 * 這個執行緒會執行 LOOP 次 data++ 操作
 */
void *addchild(void *arg) {
    int id = 0;  // 這個執行緒的 ID 是 0
    
    for (int i = 0; i < LOOP; i++) {
        lock(id);      // 取得鎖，進入臨界區
        data++;        // 臨界區操作：增加 data
        unlock(id);    // 釋放鎖，離開臨界區
    }
    return NULL;
}

/*
 * 主函數
 * 建立兩個執行緒，一個執行加法，一個執行減法
 * 理論上最終 data 應該為 0（因為各執行 LOOP 次加減）
 */
int main() {
    pthread_t t1, t2;  // 兩個執行緒的控制結構

    // 初始化共享變數
    data = 0;
    atomic_store(&turn, 0);      // 初始時輪到執行緒 0
    atomic_store(&flag[0], 0);   // 執行緒 0 初始不想進入臨界區
    atomic_store(&flag[1], 0);   // 執行緒 1 初始不想進入臨界區

    // 建立兩個執行緒
    // t1 執行 addchild（加法）
    pthread_create(&t1, NULL, addchild, NULL);
    // t2 執行 subchild（減法）
    pthread_create(&t2, NULL, subchild, NULL);

    // 等待兩個執行緒都執行完畢
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // 輸出最終結果（理論上應該為 0）
    printf("Final data (Peterson) = %d\n", data);
    return 0;
}
