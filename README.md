# Arduino Nano Claw Tank

這是一個安裝於小型載具機器人的 Arduino Nano 韌體專案。V7RC APP 透過 BLE 連接外接藍牙接收模組；模組再以 UART 將 V7RC VPP 坦克模式封包送至 Nano。Nano 解析轉向、油門、爪子與升降通道，經差速混控後驅動兩路直流馬達及舵機。

目前已提供 MVP 韌體、純 C++ 協定／控制測試及 Arduino CLI 編譯流程。實機使用前仍須確認藍牙鮑率、馬達方向及舵機安全行程。

## 系統資料流

```text
V7RC APP -- BLE --> 藍牙接收模組 -- UART --> Arduino Nano
                                                    |-- M1 / M2 馬達驅動
                                                    |-- D9 / D10 / D11 舵機
                                                    `-- D8 蜂鳴器
```

## 預設接線

| Nano 腳位 | 功能 | 外部連接 |
| --- | --- | --- |
| D2 | SoftwareSerial RX | 藍牙 TX |
| D3 | SoftwareSerial TX | 藍牙 RX |
| D4 | M1 DIR | 馬達驅動器 M1 方向 |
| D5 | M1 PWM | 馬達驅動器 M1 速度 |
| D6 | M2 PWM | 馬達驅動器 M2 速度 |
| D7 | M2 DIR | 馬達驅動器 M2 方向 |
| D8 | 蜂鳴器 | 蜂鳴器訊號 |
| D9 | Servo 1 | 爪子，channel 3 |
| D10 | Servo 2 | 上下，channel 4 |
| D11 | Servo 3 | 尚未指定，MVP 保持中立 |

所有模組必須共地。Nano I/O 為 5V 邏輯；若藍牙模組 RX 僅容許 3.3V，Nano D3 到模組 RX 之間要加入分壓器或邏輯電平轉換器。舵機與馬達不可直接由 Nano 的 5V 腳供電，應使用容量足夠的獨立電源並共地，以免重啟或損壞板子。

## V7RC VPP 協定

坦克模式封包範例：

```text
SRT1500150015001500#
```

固定長度為 20 bytes：`SRT` + 四個 4 位數通道 + `#`。

| 通道 | 範圍 | 中心 | 本專案用途 |
| --- | --- | --- | --- |
| CH1 | 1000–2000 | 1500 | 左右轉向；預設 1000 左轉、2000 右轉 |
| CH2 | 1000–2000 | 1500 | 前進／後退油門；1000 後退、2000 前進 |
| CH3 | 1000–2000 us | 1500 us | D9 爪子舵機 |
| CH4 | 1000–2000 us | 1500 us | D10 上下舵機 |

預設馬達 deadband 為 1475–1525。超過 300 ms 沒有完整有效封包時，韌體會將兩路馬達 PWM 設為 0；舵機維持最後有效位置。

CH1 與 CH2 會先轉換為轉向量與油門量，再進行差速混控：

```text
M1 = throttle + steering
M2 = throttle - steering
```

混控結果會限制在 -500 至 +500。中立油門搭配轉向可讓兩顆馬達反向運轉、使載具原地旋轉；前進搭配轉向則會提高一側、降低另一側。若實機左右相反，可切換 `STEERING_REVERSED`，不必交換 CH1/CH2。

常用控制封包與預期混控結果：

| 操作 | 封包 | M1 | M2 |
| --- | --- | ---: | ---: |
| 停止 | `SRT1500150015001500#` | 0 | 0 |
| 全速前進 | `SRT1500200015001500#` | +500 | +500 |
| 全速後退 | `SRT1500100015001500#` | -500 | -500 |
| 原地右轉（預設方向） | `SRT2000150015001500#` | +500 | -500 |
| 原地左轉（預設方向） | `SRT1000150015001500#` | -500 | +500 |
| 前進並右轉 | `SRT2000200015001500#` | +500 | 0 |

表中的左右方向以 M1 為左側、M2 為右側的安裝方式為前提。若實際車體配置相反，應校正 `M1_REVERSED`、`M2_REVERSED` 或 `STEERING_REVERSED`。

## 開發需求與函式庫

目標環境：

- Arduino Nano（經典 ATmega328P；實際版本待確認）
- Arduino AVR Boards core（FQBN：`arduino:avr:nano`）
- `Servo`：Arduino 官方舵機函式庫，控制 D9–D11
- `SoftwareSerial`：Arduino AVR core 內建，用 D2/D3 接藍牙 UART，通常不需額外安裝

若使用 Arduino CLI，可準備相依項目：

```bash
arduino-cli core install arduino:avr
arduino-cli lib install Servo
```

本工作區使用共用 Arduino CLI Docker container；可從專案根目錄執行：

```bash
/Users/louischuang/.codex/skills/arduino-cli-container/scripts/arduino-container version
/Users/louischuang/.codex/skills/arduino-cli-container/scripts/arduino-container core list
/Users/louischuang/.codex/skills/arduino-cli-container/scripts/arduino-container compile \
  --fqbn arduino:avr:nano:cpu=atmega328old \
  /workspace/ArduinoNanoClawTank
```

部分 Nano 使用新版 bootloader，這時將 FQBN 改為 `arduino:avr:nano:cpu=atmega328`。編譯不受 bootloader 選項影響太大，但實機上傳時必須選對處理器版本。

## 設定與實機校正

以下內容已集中放在 `ArduinoNanoClawTank/config.h`，以便實機校正：

- 藍牙 UART 鮑率（暫定 9600）
- 左右馬達方向反轉
- CH1 轉向方向反轉
- deadband（預設 ±25）
- failsafe timeout（預設 300 ms）
- 各舵機最小、中立、最大脈寬
- D2/D3 軟體 UART或 D0/D1 硬體 UART

設定集中於 `ArduinoNanoClawTank/config.h`。目前實作採用 D2/D3 軟體 UART；若要改用 D0/D1，需在實作層切換通訊介面，並停用同一 UART 上的除錯輸出。

## 原始碼結構

```text
ArduinoNanoClawTank/
├── ArduinoNanoClawTank.ino  # setup、主迴圈、UART 與 failsafe
├── config.h                 # 腳位與可校正參數
├── v7rc_protocol.*          # 無動態配置的逐 byte 解析器
├── control_math.*           # CH1/CH2 deadband、差速混控與 PWM 映射
└── actuators.*              # 馬達、舵機與非阻塞蜂鳴器輸出
tests/
├── test_main.cpp            # 主機端協定與控制測試
└── run_tests.sh             # 測試編譯及執行腳本
```

執行不需要 Arduino 硬體的測試：

```bash
./tests/run_tests.sh
```

目前 Nano 新、舊 bootloader FQBN 均已編譯通過。差速混控版本使用 5,532 bytes Flash（18%）及 379 bytes SRAM（18%）。

## 文件

- `AGENTS.md`：後續開發代理需遵守的架構、安全及驗證規則。
- `MVP.md`：首版可交付範圍與驗收標準。
- `TODO.md`：依相依關係排序的實作清單與待確認事項。
