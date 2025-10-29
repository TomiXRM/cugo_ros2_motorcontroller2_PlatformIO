#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <Arduino.h>

// デバッグ出力の有効/無効を切り替え
#define DEBUG_ENABLED 1

// デバッグ用シリアルポート設定

// Serial1 ← HW UART 0
// Serial2 ← HW UART 1

#if DEBUG_ENABLED
#define DEBUG_SERIAL Serial2
#define DEBUG_BAUD 115200

// デバッグ出力マクロ
#define DEBUG_PRINT(x) DEBUG_SERIAL.print(x)
#define DEBUG_PRINTLN(x) DEBUG_SERIAL.println(x)
#define DEBUG_PRINTF(fmt, ...) DEBUG_SERIAL.printf(fmt, ##__VA_ARGS__)

// 初期化関数
inline void debugInit() {
  DEBUG_SERIAL.setTX(20);
  DEBUG_SERIAL.setRX(21);
  DEBUG_SERIAL.begin(DEBUG_BAUD);
  delay(100);
}

#else
// デバッグ無効時は何もしない
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
inline void debugInit() {}
#endif

#endif  // DEBUG_PRINT_H
