# デバッグ方法

このドキュメントでは、効果的なデバッグ方法とトラブルシューティングのテクニックを説明します。

## 目次

1. [デバッグ環境のセットアップ](#デバッグ環境のセットアップ)
2. [デバッグ出力の活用](#デバッグ出力の活用)
3. [よくある問題と解決方法](#よくある問題と解決方法)
4. [シリアル通信のデバッグ](#シリアル通信のデバッグ)
5. [モーター制御のデバッグ](#モーター制御のデバッグ)
6. [エンコーダーのデバッグ](#エンコーダーのデバッグ)
7. [パフォーマンス測定](#パフォーマンス測定)

---

## デバッグ環境のセットアップ

### ハードウェア構成

```
┌──────────────┐
│   ROS2 PC    │
└──────┬───────┘
       │ USB (Serial)
       │
┌──────▼────────────────────────┐
│   Raspberry Pi Pico           │
│                               │
│  GP0 (TX) ───────┐            │
│  GND ─────────┐  │            │
└───────────────┼──┼────────────┘
                │  │
                │  │
         ┌──────▼──▼──────┐
         │ USB-Serial     │
         │ Converter      │
         │ (3.3V)         │
         └───────┬────────┘
                 │ USB
                 │
         ┌───────▼────────┐
         │  Debug PC      │
         │  (Serial Mon)  │
         └────────────────┘
```

### 必要な機器

1. **USB-シリアル変換器** (3.3V)
   - FT232RL、CP2102、CH340などのモジュール
   - **重要**: 必ず3.3Vレベルのものを使用（5Vは使用禁止）

2. **ジャンパーワイヤー** × 2
   - Pico GP0 (TX) → 変換器 RX
   - Pico GND → 変換器 GND

### 接続手順

1. **電源を切った状態で接続**

   ```shell
   Pico GP20 (Pin 26) ──→  変換器 RX
   Pico GND (Pin 23)  ──→  変換器 GND
   ```

2. **変換器をPCに接続**
   - Linuxの場合: `/dev/ttyUSB0`などとして認識される
   - Windowsの場合: `COM3`などとして認識される

3. **Picoに電源を投入**

### シリアルモニターの起動

#### Linuxの場合

```bash
# screenを使用
screen /dev/ttyUSB0 115200

# minicomを使用
minicom -D /dev/ttyUSB0 -b 115200

# PlatformIO CLIを使用
pio device monitor --port /dev/ttyUSB0 --baud 115200
```

#### Windowsの場合

- TeraTerm、PuTTY、Arduino IDEのシリアルモニターなどを使用
- ボーレート: 115200
- データビット: 8
- パリティ: なし
- ストップビット: 1

---

## デバッグ出力の活用

### ArduinoLogの基本API

```cpp
#include "DebugPrint.h"  // debugInit() でArduinoLogを初期化

Log.noticeln("System started");
Log.infoln("Target RPM received");
Log.errorln("Checksum mismatch");

// printf互換のフォーマット
Log.trace("Value: %d, Float: %.2f\n", 42, 3.14);
```

`xxx()`は改行なし、`xxxln()`は自動で改行を付与します。負荷の高いループでは`Log.trace()`や`Log.traceln()`を使うとログレベルでまとめて抑制できます。

### ログレベルの調整

ArduinoLogは7段階のログレベルをサポートします。`debugInit()`では`LOG_LEVEL_VERBOSE`を指定しているため、すべてのレベルが出力されます。必要に応じて`Log.setLevel()`で動的に変更できます。

```cpp
void setup() {
  debugInit();
  Log.setLevel(LOG_LEVEL_NOTICE);  // NOTICE以上だけ出力
  Log.setPrefix([](Print* out, int level) {
    static const char levelTag[] = {'S', 'F', 'E', 'W', 'I', 'T', 'V'};
    out->printf("[%lu ms][%c] ", millis(), levelTag[level]);
  });
}
```

### タイムスタンプ付きログ

```cpp
void logWithTimestamp(const char* message) {
  unsigned long timestamp = millis();
  Log.notice("[%lu ms] %s\n", timestamp, message);
}
```

### 条件付きデバッグ

特定の条件下でのみデバッグ出力を行う：

```cpp
void set_motor_cmd_binary(uint8_t* buf, int size, float max_rpm) {
  // ...

  if (abs(clamped_rpm.left) > max_rpm * 0.9f) {
    Log.warningln("Left RPM near limit: %.2f / %.2f",
                  clamped_rpm.left, max_rpm);
  }

  // ...
}
```

---

## よくある問題と解決方法

### 問題1: デバッグ出力が表示されない

#### チェックリスト

1. **ハードウェア接続**
  
   ```bash
   # Linuxでデバイスを確認
   ls -l /dev/ttyUSB*
   # または
   dmesg | grep tty
   ```

2. **DEBUG_ENABLEDが有効か**
  
   ```cpp
   // lib/DebugPrint/DebugPrint.h
   #define DEBUG_ENABLED 1  // 1になっているか確認
   ```

3. **ボーレート設定**
   - Pico側: 115200 baud (`DebugPrint.h`の`DEBUG_BAUD`）
   - シリアルモニター側: 115200 baud

4. **電圧レベル**
   - USB-シリアル変換器が3.3V動作か確認
   - 5Vの変換器を使うと通信できない、または故障の原因に

#### デバッグコード

`setup()`関数の最初に以下を追加して確認：

```cpp
void setup() {
  debugInit();

  // デバッグ出力テスト
  for(int i = 0; i < 5; i++) {
    Log.traceln("Debug test %d", i);
    delay(500);
  }

  // 以降の処理...
}
```

### 問題2: モーターが動かない

#### デバッグ手順

1. **RPM指令値の確認**
   
   ```cpp
   void set_motor_cmd_binary(uint8_t* buf, int size, float max_rpm) {
     Log.traceln("Received RPM: L=%.2f R=%.2f",
                 reciev_rpm.left, reciev_rpm.right);
     Log.traceln("Clamped RPM: L=%.2f R=%.2f",
                 clamped_rpm.left, clamped_rpm.right);
     motorController->setRPM(clamped_rpm.left, clamped_rpm.right);
   }
   ```

2. **フェイルセーフ状態の確認**
   
   ```cpp
   void check_failsafe() {
     COM_FAIL_COUNT++;
     Log.traceln("COM_FAIL_COUNT: %d", COM_FAIL_COUNT);

     if (COM_FAIL_COUNT > 5) {
       Log.warningln("FAILSAFE TRIGGERED - Stopping motor");
       stop_motor_immediately();
     }
   }
   ```

3. **モーターコントローラーの状態確認**
   
   ```cpp
   void CugoMotorController::setRPM(float left_rpm, float right_rpm) {
     Log.traceln("CugoMotorController::setRPM(%.2f, %.2f)",
                 left_rpm, right_rpm);
     cugo_rpm_direct_instructions(left_rpm, right_rpm);
   }
   ```

### 問題3: エンコーダーカウントが増えない

#### CugoSDK使用時

```cpp
void CugoMotorController::update() {
  Log.traceln("CugoMotorController::update() called");
  ld2_get_cmd();

  // エンコーダー値を確認
  Log.traceln("Encoder L: %ld, R: %ld",
              cugo_current_count_L, cugo_current_count_R);
}
```

#### 汎用DCモータ使用時

```cpp
void GenericMotorController::encoderISR_LeftA() {
  // 割り込みハンドラ内では最小限のデバッグのみ
  static unsigned long last_debug = 0;
  if (millis() - last_debug > 1000) {  // 1秒に1回だけ
    Log.trace("Encoder L count: %ld\n", encoder_count_left_);
    last_debug = millis();
  }

  // エンコーダー処理...
}
```

---

## シリアル通信のデバッグ

### パケット受信のモニタリング

```cpp
void onSerialPacketReceived(const uint8_t* buffer, size_t size) {
  Log.trace("Packet received: size=%d\n", size);

  // ヘッダーのダンプ
  Log.trace("Header: ");
  for(int i = 0; i < SERIAL_HEADER_SIZE; i++) {
    Log.trace("%02X ", buffer[i]);
  }
  Log.traceln("");

  // チェックサム検証
  uint16_t recv_checksum = read_uint16_t_from_header(tempBuffer, RECV_HEADER_CHECKSUM_PTR);
  uint16_t calc_checksum = calculate_checksum(body_ptr, SERIAL_BIN_BUFF_SIZE);

  Log.trace("Checksum: recv=0x%04X calc=0x%04X %s\n",
            recv_checksum, calc_checksum,
            (recv_checksum == calc_checksum) ? "OK" : "NG");

  // 以降の処理...
}
```

### パケット送信のモニタリング

```cpp
void onSerialPacketReceived(const uint8_t* buffer, size_t size) {
  // ...（受信処理）

  // 送信パケットの確認
  Log.trace("Sending encoder: L=%ld R=%ld\n", encoder_L, encoder_R);

  packetSerial.send(send_packet, send_len);
  Log.traceln("Packet sent");
}
```

### バッファオーバーフローの検出

```cpp
void loop() {
  // ...

  packetSerial.update();

  if (packetSerial.overflow()) {
    Log.errorln("PacketSerial overflow detected!");
  }
}
```

---

## モーター制御のデバッグ

### RPMクランプの動作確認

```cpp
MotorRPM clamp_rpm_rotation_priority(MotorRPM target_rpm, float max_rpm) {
  Log.trace("Before clamp: L=%.2f R=%.2f\n",
            target_rpm.left, target_rpm.right);

  // クランプ処理...

  Log.trace("After clamp: L=%.2f R=%.2f\n",
            new_rpm.left, new_rpm.right);

  return new_rpm;
}
```

### 周期タスクの実行確認

```cpp
void loop() {
  static unsigned long last_print_10ms = 0;
  static unsigned long last_print_100ms = 0;

  current_time = micros();

  if (current_time - prev_time_10ms > 10000) {
    job_10ms();
    prev_time_10ms = current_time;

    // 10秒に1回だけ出力
    if(millis() - last_print_10ms > 10000) {
      Log.traceln("10ms job executed");
      last_print_10ms = millis();
    }
  }

  if (current_time - prev_time_100ms > 100000) {
    job_100ms();
    prev_time_100ms = current_time;

    // 毎回出力（100msなので負荷は軽い）
    Log.traceln("100ms job executed");
  }

  // ...
}
```

---

## エンコーダーのデバッグ

### エンコーダー信号の確認

```cpp
void GenericMotorController::init() {
  // ...

  // エンコーダーの初期状態を確認
  bool la = digitalRead(left_enc_a_pin_);
  bool lb = digitalRead(left_enc_b_pin_);
  bool ra = digitalRead(right_enc_a_pin_);
  bool rb = digitalRead(right_enc_b_pin_);

  Log.trace("Encoder initial state: LA=%d LB=%d RA=%d RB=%d\n",
            la, lb, ra, rb);
}
```

### エンコーダーカウントの変化監視

```cpp
void job_100ms() {
  static long prev_enc_L = 0;
  static long prev_enc_R = 0;

  check_failsafe();

  if (motorController) {
    motorController->update();

    long enc_L = motorController->getEncoderCountLeft();
    long enc_R = motorController->getEncoderCountRight();

    // カウントの変化量を表示
    Log.trace("Encoder delta: L=%ld R=%ld\n",
              enc_L - prev_enc_L, enc_R - prev_enc_R);

    prev_enc_L = enc_L;
    prev_enc_R = enc_R;
  }
}
```

### 割り込みハンドラのデバッグ

割り込みハンドラ内でのデバッグ出力は最小限に！多すぎると割り込み処理時間が長くなり、パルスを取りこぼします。

```cpp
void GenericMotorController::encoderISR_LeftA() {
  encoder_count_left_++;

  // 1000カウントごとにデバッグ出力
  if (encoder_count_left_ % 1000 == 0) {
    Log.trace("L: %ld\n", encoder_count_left_);
  }
}
```

---

## パフォーマンス測定

### ループ実行時間の測定

```cpp
void loop() {
  static unsigned long loop_count = 0;
  static unsigned long last_report = 0;

  unsigned long loop_start = micros();

  // メインループ処理...

  unsigned long loop_end = micros();
  unsigned long loop_time = loop_end - loop_start;

  loop_count++;

  // 1秒ごとに統計を表示
  if (millis() - last_report > 1000) {
    Log.trace("Loop stats: count=%lu avg_time=%lu us\n",
              loop_count, loop_time);
    loop_count = 0;
    last_report = millis();
  }
}
```

### 関数実行時間の測定

```cpp
class Timer {
public:
  Timer(const char* name) : name_(name), start_(micros()) {}

  ~Timer() {
    unsigned long elapsed = micros() - start_;
    Log.trace("%s took %lu us\n", name_, elapsed);
  }

private:
  const char* name_;
  unsigned long start_;
};

// 使用例
void expensiveFunction() {
  Timer t("expensiveFunction");

  // 処理...
}
```

### メモリ使用量の確認

```cpp
void printMemoryInfo() {
  extern char __StackLimit, __bss_end__;
  extern "C" char* sbrk(int);

  char* heap_end = (char*)sbrk(0);
  int free_memory = &__StackLimit - heap_end;

  Log.trace("Free memory: %d bytes\n", free_memory);
}
```

---

## 高度なデバッグテクニック

### 1. リングバッファによるイベントログ

割り込みハンドラなど、即座にデバッグ出力できない場所でのログ記録：

```cpp
#define LOG_BUFFER_SIZE 32

struct LogEntry {
  unsigned long timestamp;
  uint8_t event_type;
  int32_t value;
};

LogEntry log_buffer[LOG_BUFFER_SIZE];
volatile int log_index = 0;

void log_event(uint8_t type, int32_t value) {
  int idx = log_index % LOG_BUFFER_SIZE;
  log_buffer[idx].timestamp = micros();
  log_buffer[idx].event_type = type;
  log_buffer[idx].value = value;
  log_index++;
}

void print_log() {
  Log.noticeln("Event Log:");
  for(int i = 0; i < LOG_BUFFER_SIZE; i++) {
    Log.notice("  [%lu] Type:%d Value:%ld\n",
               log_buffer[i].timestamp,
               log_buffer[i].event_type,
               log_buffer[i].value);
  }
}
```

### 2. アサーション

```cpp
#if DEBUG_ENABLED
  #define ASSERT(condition, message) \
    if(!(condition)) { \
      Log.fatalln("ASSERT FAILED: %s at %s:%d", \
                  message, __FILE__, __LINE__); \
      while(1); /* 停止 */ \
    }
#else
  #define ASSERT(condition, message)
#endif

// 使用例
void setRPM(float rpm) {
  ASSERT(rpm >= -200 && rpm <= 200, "RPM out of range");
  // 処理...
}
```

### 3. リモートデバッグコマンド

シリアル経由でコマンドを送ってデバッグ情報を取得：

```cpp
void processDebugCommand() {
  if (DEBUG_SERIAL.available()) {
    char cmd = DEBUG_SERIAL.read();

    switch(cmd) {
      case 's':  // Status
        Log.notice("Encoder L:%ld R:%ld\n",
                   motorController->getEncoderCountLeft(),
                   motorController->getEncoderCountRight());
        break;

      case 'm':  // Memory
        printMemoryInfo();
        break;

      case 'l':  // Log
        print_log();
        break;

      case 'r':  // Reset
        Log.warningln("Resetting...");
        delay(100);
        watchdog_reboot(0, 0, 0);
        break;
    }
  }
}
```

---

## まとめ

効果的なデバッグのポイント：

1. **デバッグ出力を積極的に使う** - ROS2通信と独立したSerial2により、いつでもデバッグ可能
2. **段階的にデバッグ** - 一度に多くのコードを追加せず、少しずつ確認
3. **データを可視化** - エンコーダーカウント、RPM値など、数値をリアルタイムで確認
4. **パフォーマンスに注意** - 割り込みハンドラ内では最小限のデバッグのみ
5. **条件付きデバッグ** - 必要な時だけ出力して、ログが流れすぎないように

---

[← 使い方](usage.md) | [README →](README.md)
