/*
 * Semaphore（號誌）實作
 * 使用 semaphore 來實現互斥鎖，確保同一時間只有一個執行緒能進入臨界區
 * 這裡實作的是 binary semaphore（二元號誌），值只能是 0 或 1
 */

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

/*
 * 全域資料結構
 * data: 共享的資料，兩個執行緒會同時存取
 * count: semaphore 的計數值，表示可用資源的數量
 *        count > 0 表示有資源可用，count = 0 表示沒有資源
 * m: 用於保護 count 變數的 spinlock（自旋鎖）
 *    因為多個執行緒會同時修改 count，需要這個鎖來保護
 */
typedef struct global_data {
    int data;          // 共用的資料
    int count;         // semaphore 的計數值（可用資源數量）
    atomic_flag m;     // 保護 count 的 spinlock（自旋鎖）
} global_data;

// 全域資料實例
global_data gdata;

// 每個執行緒執行的迴圈次數
#define LOOP 100000

/*
 * 取得 spinlock（自旋鎖）
 * 使用 atomic_flag_test_and_set 來原子性地測試並設定鎖
 * 如果鎖已被其他執行緒持有，則持續等待（busy wait）
 */
static void lock_spin() {
    // atomic_flag_test_and_set 會原子性地：
    // 1. 檢查 flag 是否為 0（未鎖定）
    // 2. 如果是 0，則設為 1（鎖定）並返回 0（成功取得鎖）
    // 3. 如果是 1（已鎖定），則返回 1（需要等待）
    while (atomic_flag_test_and_set(&gdata.m)) {
        // busy wait: 持續嘗試取得鎖，直到成功
    }
}

/*
 * 釋放 spinlock（自旋鎖）
 * 將鎖標記清除，讓其他等待的執行緒可以取得鎖
 */
static void unlock_spin() {
    // 清除鎖標記，表示釋放鎖
    atomic_flag_clear(&gdata.m);
}

/*
 * Semaphore 的 P 操作（wait 操作）
 * 功能：等待直到有可用資源（count > 0），然後取得資源（count--）
 * 
 * 運作方式：
 * 1. 取得保護 count 的鎖
 * 2. 檢查 count 是否大於 0
 * 3. 如果 count > 0，則 count-- 並釋放鎖，成功取得資源
 * 4. 如果 count = 0，則釋放鎖並繼續等待下一輪
 */
void semwait() {
    while (1) {
        // 取得保護 count 的鎖
        lock_spin();
        
        // 檢查是否有可用資源
        if (gdata.count > 0) {
            // 有資源可用，取得資源（count--）
            gdata.count--;
            unlock_spin();
            break;  // 成功取得資源，離開迴圈
        }
        
        // 沒有資源可用，釋放鎖，等待下一輪再試
        unlock_spin();
        // busy wait: 持續嘗試取得資源
        // 注意：這裡可以加入 sleep 來減少 CPU 使用率
    }
}

/*
 * Semaphore 的 V 操作（signal 操作）
 * 功能：釋放資源（count++），讓等待的執行緒可以取得資源
 * 
 * 運作方式：
 * 1. 取得保護 count 的鎖
 * 2. 增加 count（釋放資源）
 * 3. 釋放鎖
 */
void semsignal() {
    // 取得保護 count 的鎖
    lock_spin();
    // 釋放資源（count++）
    gdata.count++;
    // 釋放鎖
    unlock_spin();
}

/*
 * 執行緒函數：執行加法操作
 * 這個執行緒會執行 LOOP 次 data++ 操作
 * 使用 semaphore 來保護臨界區（data 的修改）
 */
void *addchild(void *arg) {
    for (int i = 0; i < LOOP; i++) {
        semwait();        // P 操作：等待並取得資源（進入臨界區）
        gdata.data++;     // 臨界區操作：增加 data
        semsignal();      // V 操作：釋放資源（離開臨界區）
    }
    return NULL;
}

/*
 * 執行緒函數：執行減法操作
 * 這個執行緒會執行 LOOP 次 data-- 操作
 * 使用 semaphore 來保護臨界區（data 的修改）
 */
void *subchild(void *arg) {
    for (int i = 0; i < LOOP; i++) {
        semwait();        // P 操作：等待並取得資源（進入臨界區）
        gdata.data--;     // 臨界區操作：減少 data
        semsignal();      // V 操作：釋放資源（離開臨界區）
    }
    return NULL;
}

/*
 * 主函數
 * 建立兩個執行緒，一個執行加法，一個執行減法
 * 使用 binary semaphore（count = 1）來確保互斥存取
 * 理論上最終 data 應該為 0（因為各執行 LOOP 次加減）
 */
int main() {
    pthread_t t1, t2;  // 兩個執行緒的控制結構

    // 初始化全域資料
    gdata.data  = 0;                    // 初始資料值為 0
    gdata.count = 1;                    // binary semaphore = 1（只有一個資源可用）
    atomic_flag_clear(&gdata.m);        // 初始化 spinlock（清除鎖標記）

    // 建立兩個執行緒
    // t1 執行 addchild（加法）
    pthread_create(&t1, NULL, addchild, NULL);
    // t2 執行 subchild（減法）
    pthread_create(&t2, NULL, subchild, NULL);

    // 等待兩個執行緒都執行完畢
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // 輸出最終結果（理論上應該為 0）
    printf("Final data (Semaphore) = %d\n", gdata.data);
    return 0;
}
