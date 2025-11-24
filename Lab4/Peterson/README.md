# Peterson 演算法實作

## 專案簡介

本專案實作了 **Peterson 演算法**，這是一個用於兩個執行緒的互斥鎖演算法。Peterson 演算法的特點是：
- 不需要硬體支援的特殊指令（如 test-and-set）
- 僅使用兩個變數（`flag` 和 `turn`）就能實現互斥
- 保證互斥（Mutual Exclusion）、進展（Progress）和有限等待（Bounded Waiting）

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

### 共享變數

- `data`: 共享的資料，兩個執行緒會同時存取
- `turn`: 表示目前輪到哪個執行緒進入臨界區（0 或 1）
- `flag[2]`: 表示執行緒是否想要進入臨界區
  - `flag[i] = 1` 表示執行緒 i 想要進入臨界區
  - `flag[i] = 0` 表示執行緒 i 不想進入臨界區

### 核心函數

- `lock(int id)`: Peterson 演算法的鎖定函數
- `unlock(int id)`: Peterson 演算法的解鎖函數
- `addchild()`: 執行緒函數，執行加法操作（執行緒 ID = 0）
- `subchild()`: 執行緒函數，執行減法操作（執行緒 ID = 1）

## 完整運作流程

### 1. 程式初始化階段

```
main() 函數開始執行：
├─ 初始化 data = 0
├─ 初始化 turn = 0（初始時輪到執行緒 0）
├─ 初始化 flag[0] = 0（執行緒 0 初始不想進入臨界區）
└─ 初始化 flag[1] = 0（執行緒 1 初始不想進入臨界區）
```

### 2. 執行緒建立階段

```
main() 建立兩個執行緒：
├─ pthread_create(&t1, NULL, addchild, NULL)  → 建立執行緒 0（加法執行緒）
└─ pthread_create(&t2, NULL, subchild, NULL)  → 建立執行緒 1（減法執行緒）

此時兩個執行緒開始並行執行
```

### 3. 執行緒執行階段（並行執行）

#### 執行緒 0（addchild）的執行流程：

```
for (i = 0; i < LOOP; i++):
├─ 呼叫 lock(0) 嘗試進入臨界區
│  ├─ 設定 flag[0] = 1（宣告我想進入）
│  ├─ 設定 turn = 1（禮讓執行緒 1）
│  └─ 等待條件：while (flag[1] && turn == 1)
│     └─ 如果執行緒 1 也想進入且輪到它，則等待
│     └─ 否則進入臨界區
├─ 執行臨界區操作：data++
└─ 呼叫 unlock(0)
   └─ 設定 flag[0] = 0（表示不再需要進入）
```

#### 執行緒 1（subchild）的執行流程：

```
for (i = 0; i < LOOP; i++):
├─ 呼叫 lock(1) 嘗試進入臨界區
│  ├─ 設定 flag[1] = 1（宣告我想進入）
│  ├─ 設定 turn = 0（禮讓執行緒 0）
│  └─ 等待條件：while (flag[0] && turn == 0)
│     └─ 如果執行緒 0 也想進入且輪到它，則等待
│     └─ 否則進入臨界區
├─ 執行臨界區操作：data--
└─ 呼叫 unlock(1)
   └─ 設定 flag[1] = 0（表示不再需要進入）
```

### 4. Peterson 演算法的關鍵運作機制

#### lock() 函數的三個步驟：

**步驟 1：宣告意圖**
```
atomic_store(&flag[id], 1)
```
- 執行緒宣告「我想要進入臨界區」
- 這是一個原子操作，確保其他執行緒能看到這個狀態

**步驟 2：禮讓對方**
```
atomic_store(&turn, other)
```
- 將 turn 設為對方的 ID，表示「讓對方優先」
- 這是 Peterson 演算法的關鍵：即使我先宣告，也讓對方有機會先進入

**步驟 3：等待條件**
```
while (atomic_load(&flag[other]) && atomic_load(&turn) == other)
```
- 等待條件：當對方也想進入（flag[other] = 1）且輪到對方（turn = other）時，持續等待
- 只有當以下任一條件成立時，才能進入臨界區：
  - 對方不想進入（flag[other] = 0）
  - 輪到我（turn = id）

### 5. 典型執行場景分析

#### 場景 1：兩個執行緒同時想進入臨界區

```
時間軸：
T1: 執行緒 0 執行 lock(0)
    ├─ flag[0] = 1
    ├─ turn = 1（禮讓執行緒 1）
    └─ 檢查：flag[1] = ? 且 turn = ?

T2: 執行緒 1 執行 lock(1)
    ├─ flag[1] = 1
    ├─ turn = 0（禮讓執行緒 0）
    └─ 檢查：flag[0] = 1 且 turn = 0 → 等待

結果：執行緒 0 可以進入（因為 turn = 0，輪到它）
      執行緒 1 必須等待（因為 turn = 0，輪到執行緒 0）
```

#### 場景 2：只有一個執行緒想進入

```
時間軸：
T1: 執行緒 0 執行 lock(0)
    ├─ flag[0] = 1
    ├─ turn = 1
    └─ 檢查：flag[1] = 0 → 直接進入（對方不想進入）

結果：執行緒 0 立即進入臨界區，無需等待
```

### 6. 互斥保證機制

Peterson 演算法保證互斥的原理：

1. **互斥性（Mutual Exclusion）**
   - 如果兩個執行緒都想進入，turn 變數會決定誰先進入
   - 只有一個執行緒能通過 while 迴圈的檢查

2. **進展性（Progress）**
   - 如果只有一個執行緒想進入，它不會被阻塞
   - 如果兩個執行緒都想進入，turn 變數確保至少有一個能進入

3. **有限等待（Bounded Waiting）**
   - 透過 turn 變數的輪流機制，確保等待的執行緒不會無限期等待
   - 最多等待一次對方執行完臨界區

### 7. 程式結束階段

```
main() 等待兩個執行緒完成：
├─ pthread_join(t1, NULL)  → 等待執行緒 0 完成（執行 LOOP 次加法）
└─ pthread_join(t2, NULL)  → 等待執行緒 1 完成（執行 LOOP 次減法）

最終結果：
└─ 輸出 "Final data (Peterson) = 0"
   （理論上應該為 0，因為各執行 LOOP 次加減）
```

## 預期結果

程式執行後應該輸出：
```
Final data (Peterson) = 0
```

如果結果不是 0，表示互斥鎖實作有問題，導致 race condition。

## 注意事項

1. **原子操作的重要性**
   - 使用 `atomic_store` 和 `atomic_load` 確保變數的讀寫是原子性的
   - 如果沒有原子操作，可能會出現 race condition

2. **Busy Wait**
   - `lock()` 函數中的 while 迴圈是 busy wait（忙等待）
   - 這會持續消耗 CPU 資源，在實際應用中可能需要加入 sleep 來減少 CPU 使用率

3. **僅適用於兩個執行緒**
   - Peterson 演算法只適用於兩個執行緒的情況
   - 對於多個執行緒，需要使用其他演算法（如 Bakery 演算法）

## 學習重點

- 理解 Peterson 演算法的三個步驟
- 理解 `flag` 和 `turn` 變數的作用
- 理解互斥、進展和有限等待的保證機制
- 理解原子操作在多執行緒程式中的重要性

