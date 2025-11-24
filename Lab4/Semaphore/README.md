# Semaphore（號誌）實作

## 專案簡介

本專案實作了 **Semaphore（號誌）** 機制，使用 semaphore 來實現互斥鎖，確保同一時間只有一個執行緒能進入臨界區。這裡實作的是 **binary semaphore（二元號誌）**，其計數值只能是 0 或 1。

Semaphore 是作業系統中重要的同步機制，由 Dijkstra 提出，使用 P（wait）和 V（signal）操作來控制資源的存取。

## 編譯方式

```bash
./compile.sh
```

或手動編譯：
```bash
gcc -pthread app.c -o app
```

## 執行方式

```bash
./app
```

## 程式結構

### 全域資料結構

```c
typedef struct global_data {
    int data;          // 共享的資料
    int count;         // semaphore 的計數值（可用資源數量）
    atomic_flag m;     // 保護 count 的 spinlock（自旋鎖）
} global_data;
```

- `data`: 共享的資料，兩個執行緒會同時存取
- `count`: semaphore 的計數值，表示可用資源的數量
  - `count > 0` 表示有資源可用
  - `count = 0` 表示沒有資源
- `m`: 用於保護 `count` 變數的 spinlock（自旋鎖）

### 核心函數

- `lock_spin()`: 取得 spinlock（自旋鎖）
- `unlock_spin()`: 釋放 spinlock
- `semwait()`: Semaphore 的 P 操作（wait 操作）
- `semsignal()`: Semaphore 的 V 操作（signal 操作）
- `addchild()`: 執行緒函數，執行加法操作
- `subchild()`: 執行緒函數，執行減法操作

## 完整運作流程

### 1. 程式初始化階段

```
main() 函數開始執行：
├─ 初始化 gdata.data = 0（共享資料初始值）
├─ 初始化 gdata.count = 1（binary semaphore，只有一個資源可用）
└─ 初始化 gdata.m（清除 spinlock 標記，表示鎖未被持有）
```

**關鍵點**：`count = 1` 表示這是一個 binary semaphore，同一時間只允許一個執行緒進入臨界區。

### 2. 執行緒建立階段

```
main() 建立兩個執行緒：
├─ pthread_create(&t1, NULL, addchild, NULL)  → 建立加法執行緒
└─ pthread_create(&t2, NULL, subchild, NULL)  → 建立減法執行緒

此時兩個執行緒開始並行執行
```

### 3. Spinlock 運作機制

在深入 semaphore 之前，先理解 spinlock 的運作：

#### lock_spin() 運作流程：

```
lock_spin():
├─ 呼叫 atomic_flag_test_and_set(&gdata.m)
│  ├─ 原子性地檢查 flag 是否為 0（未鎖定）
│  ├─ 如果為 0：設為 1（鎖定）並返回 0（成功取得鎖）
│  └─ 如果為 1（已鎖定）：返回 1（需要等待）
└─ 如果返回 1，則持續迴圈（busy wait），直到成功取得鎖
```

#### unlock_spin() 運作流程：

```
unlock_spin():
└─ 呼叫 atomic_flag_clear(&gdata.m)
   └─ 清除鎖標記，表示釋放鎖
```

**為什麼需要 spinlock？**
- `count` 變數會被多個執行緒同時修改
- 需要 spinlock 來保護 `count` 的讀寫操作，確保原子性

### 4. Semaphore 的 P 操作（semwait）

#### semwait() 完整運作流程：

```
semwait()（P 操作）：
└─ while (1) 無限迴圈，直到成功取得資源
   ├─ 步驟 1：取得保護 count 的鎖
   │  └─ lock_spin() → 取得 spinlock
   │
   ├─ 步驟 2：檢查是否有可用資源
   │  └─ if (gdata.count > 0)
   │     ├─ 情況 A：有資源可用（count > 0）
   │     │  ├─ gdata.count--（取得資源，減少計數）
   │     │  ├─ unlock_spin()（釋放 spinlock）
   │     │  └─ break（成功取得資源，離開迴圈）
   │     │
   │     └─ 情況 B：沒有資源可用（count = 0）
   │        ├─ unlock_spin()（釋放 spinlock，讓其他執行緒有機會）
   │        └─ 繼續迴圈，等待下一輪再試
   │
   └─ 步驟 3：如果沒有資源，持續嘗試（busy wait）
```

**關鍵設計點**：
- 如果沒有資源（count = 0），必須先釋放 spinlock，再繼續等待
- 如果不釋放鎖就等待，會造成死鎖（其他執行緒無法釋放資源）

### 5. Semaphore 的 V 操作（semsignal）

#### semsignal() 完整運作流程：

```
semsignal()（V 操作）：
├─ 步驟 1：取得保護 count 的鎖
│  └─ lock_spin() → 取得 spinlock
│
├─ 步驟 2：釋放資源
│  └─ gdata.count++（增加計數，表示釋放一個資源）
│
└─ 步驟 3：釋放鎖
   └─ unlock_spin() → 釋放 spinlock
```

**關鍵設計點**：
- V 操作必須是原子性的，使用 spinlock 保護
- 釋放資源後，等待的執行緒（在 semwait 中）就能取得資源

### 6. 執行緒執行階段（並行執行）

#### 執行緒 0（addchild）的執行流程：

```
for (i = 0; i < LOOP; i++):
├─ 步驟 1：呼叫 semwait()（P 操作）
│  └─ 等待直到 count > 0，然後 count--
│     └─ 成功後進入臨界區
│
├─ 步驟 2：執行臨界區操作
│  └─ gdata.data++（增加共享資料）
│
└─ 步驟 3：呼叫 semsignal()（V 操作）
   └─ count++（釋放資源，讓其他執行緒可以進入）
```

#### 執行緒 1（subchild）的執行流程：

```
for (i = 0; i < LOOP; i++):
├─ 步驟 1：呼叫 semwait()（P 操作）
│  └─ 等待直到 count > 0，然後 count--
│     └─ 成功後進入臨界區
│
├─ 步驟 2：執行臨界區操作
│  └─ gdata.data--（減少共享資料）
│
└─ 步驟 3：呼叫 semsignal()（V 操作）
   └─ count++（釋放資源，讓其他執行緒可以進入）
```

### 7. 典型執行場景分析

#### 場景 1：兩個執行緒同時想進入臨界區

```
初始狀態：count = 1

時間軸：
T1: 執行緒 0 呼叫 semwait()
    ├─ lock_spin() → 取得 spinlock
    ├─ 檢查：count = 1 > 0 → 有資源
    ├─ count-- → count = 0（取得資源）
    ├─ unlock_spin() → 釋放 spinlock
    └─ 進入臨界區，執行 data++

T2: 執行緒 1 呼叫 semwait()
    ├─ lock_spin() → 取得 spinlock
    ├─ 檢查：count = 0 → 沒有資源
    ├─ unlock_spin() → 釋放 spinlock
    └─ 繼續迴圈等待（busy wait）

T3: 執行緒 0 完成臨界區，呼叫 semsignal()
    ├─ lock_spin() → 取得 spinlock
    ├─ count++ → count = 1（釋放資源）
    └─ unlock_spin() → 釋放 spinlock

T4: 執行緒 1 再次嘗試 semwait()
    ├─ lock_spin() → 取得 spinlock
    ├─ 檢查：count = 1 > 0 → 有資源
    ├─ count-- → count = 0（取得資源）
    ├─ unlock_spin() → 釋放 spinlock
    └─ 進入臨界區，執行 data--
```

#### 場景 2：執行緒交替執行

```
執行流程示意：
執行緒 0: semwait() → data++ → semsignal()
執行緒 1: semwait() → data-- → semsignal()
執行緒 0: semwait() → data++ → semsignal()
執行緒 1: semwait() → data-- → semsignal()
...

結果：兩個執行緒交替進入臨界區，確保互斥
```

### 8. 互斥保證機制

Semaphore 保證互斥的原理：

1. **Binary Semaphore 的特性**
   - `count = 1` 表示只有一個資源可用
   - 當一個執行緒取得資源（count--），`count` 變為 0
   - 其他執行緒必須等待，直到資源被釋放（count++）

2. **原子性保護**
   - 使用 spinlock 保護 `count` 的讀寫操作
   - 確保 `count--` 和 `count++` 是原子性的
   - 避免 race condition

3. **等待機制**
   - 當 `count = 0` 時，執行緒會在 `semwait()` 中等待
   - 當其他執行緒執行 `semsignal()` 時，`count` 變為 1
   - 等待的執行緒就能取得資源並進入臨界區

### 9. 程式結束階段

```
main() 等待兩個執行緒完成：
├─ pthread_join(t1, NULL)  → 等待加法執行緒完成（執行 LOOP 次）
└─ pthread_join(t2, NULL)  → 等待減法執行緒完成（執行 LOOP 次）

最終結果：
└─ 輸出 "Final data (Semaphore) = 0"
   （理論上應該為 0，因為各執行 LOOP 次加減）
```

## 預期結果

程式執行後應該輸出：
```
Final data (Semaphore) = 0
```

如果結果不是 0，表示 semaphore 實作有問題，導致 race condition。

## Semaphore vs Peterson 演算法

| 特性 | Semaphore | Peterson 演算法 |
|------|-----------|----------------|
| 適用執行緒數 | 任意數量 | 僅兩個執行緒 |
| 實作複雜度 | 較複雜（需要 spinlock） | 較簡單 |
| 資源概念 | 有資源計數概念 | 無資源概念 |
| 擴展性 | 可擴展為 counting semaphore | 無法擴展 |

## 注意事項

1. **Spinlock 的重要性**
   - 必須使用 spinlock 保護 `count` 變數
   - 沒有 spinlock 保護會導致 race condition

2. **釋放鎖的時機**
   - 在 `semwait()` 中，如果沒有資源，必須先釋放 spinlock 再等待
   - 否則會造成死鎖

3. **Busy Wait**
   - `semwait()` 和 `lock_spin()` 都使用 busy wait
   - 在實際應用中可能需要加入 sleep 來減少 CPU 使用率

4. **Binary vs Counting Semaphore**
   - 本實作是 binary semaphore（count = 1）
   - 可以修改 `count` 的初始值來實作 counting semaphore

## 學習重點

- 理解 Semaphore 的 P（wait）和 V（signal）操作
- 理解 spinlock 的作用和實作方式
- 理解為什麼需要 spinlock 來保護 `count` 變數
- 理解 binary semaphore 如何保證互斥
- 理解 busy wait 的機制和影響

