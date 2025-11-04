#ifndef DEBUG_PRINT_H
#define DEBUG_PRINT_H

#include <Arduino.h>
#include <ArduinoLog.h>

// デバッグ出力の有効/無効を切り替え
#define DEBUG_ENABLED 1

// デバッグ用シリアルポート設定
// Serial1 ← HW UART 0
// Serial2 ← HW UART 1

#if DEBUG_ENABLED
#define DEBUG_SERIAL Serial2
#define DEBUG_BAUD 115200

#define DEBUG_PRINTF(fmt, ...) DEBUG_SERIAL.printf(fmt, ##__VA_ARGS__)
#define DEBUG_NOTICE(fmt, ...) DEBUG_SERIAL.printf("[NOTICE] " fmt, ##__VA_ARGS__)
#define DEBUG_INFO(fmt, ...) DEBUG_SERIAL.printf("[INFO] " fmt, ##__VA_ARGS__)
#define DEBUG_TRACE(fmt, ...) DEBUG_SERIAL.printf("[TRACE] " fmt, ##__VA_ARGS__)
#define DEBUG_WARNING(fmt, ...) DEBUG_SERIAL.printf("[WARNING] " fmt, ##__VA_ARGS__)
#define DEBUG_ERROR(fmt, ...) DEBUG_SERIAL.printf("[ERROR] " fmt, ##__VA_ARGS__)

// Arduino Logラッパーマクロ（後方互換性のため残す）
#define DEBUG_PRINT(x) Log.notice(x)
#define DEBUG_PRINTLN(x) Log.noticeln(x)

// 初期化関数
inline void debugInit() {
  DEBUG_SERIAL.setTX(20);
  DEBUG_SERIAL.setRX(21);
  DEBUG_SERIAL.begin(DEBUG_BAUD);
  delay(100);

  // Arduino Log初期化
  Log.begin(LOG_LEVEL_VERBOSE, &DEBUG_SERIAL);
  Log.setPrefix([](Print* _logOutput, int logLevel) {
    const char* levelStr[] = {"SILENT", "FATAL", "ERROR", "WARN", "INFO", "TRACE", "VERB"};
    _logOutput->printf("[%s] ", levelStr[logLevel]);
  });
}

#else
// デバッグ無効時は何もしない
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(fmt, ...)
#define DEBUG_NOTICE(fmt, ...)
#define DEBUG_INFO(fmt, ...)
#define DEBUG_TRACE(fmt, ...)
#define DEBUG_WARNING(fmt, ...)
#define DEBUG_ERROR(fmt, ...)
inline void debugInit() {}
#endif

#endif  // DEBUG_PRINT_H
