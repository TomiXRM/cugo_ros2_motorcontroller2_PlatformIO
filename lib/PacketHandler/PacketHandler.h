#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <Arduino.h>

// パケット定義
#define SERIAL_BIN_BUFF_SIZE 64
#define SERIAL_HEADER_SIZE 8

// ヘッダーオフセット
#define RECV_HEADER_PRODUCT_ID_PTR 0
#define RECV_HEADER_CHECKSUM_PTR 6

// ボディオフセット（受信）
#define TARGET_RPM_L_PTR 0
#define TARGET_RPM_R_PTR 4

// ボディオフセット（送信）
#define SEND_ENCODER_L_PTR 0
#define SEND_ENCODER_R_PTR 4

/**
 * @brief シリアルパケットの処理を行うクラス
 */
class PacketHandler {
public:
  /**
   * @brief バッファにfloat値を書き込む
   */
  static void writeFloatToBuf(uint8_t* buf, const int target, float val);

  /**
   * @brief バッファにint値を書き込む
   */
  static void writeIntToBuf(uint8_t* buf, const int target, int val);

  /**
   * @brief バッファにbool値を書き込む
   */
  static void writeBoolToBuf(uint8_t* buf, const int target, bool val);

  /**
   * @brief バッファからfloat値を読み込む
   */
  static float readFloatFromBuf(uint8_t* buf, const int target);

  /**
   * @brief バッファからint値を読み込む
   */
  static int readIntFromBuf(uint8_t* buf, const int target);

  /**
   * @brief バッファからbool値を読み込む
   */
  static bool readBoolFromBuf(uint8_t* buf, const int target);

  /**
   * @brief バッファからuint8_t値を読み込む
   */
  static uint8_t readUint8FromBuf(uint8_t* buf, const int target);

  /**
   * @brief ヘッダーからuint16_t値を読み込む
   */
  static uint16_t readUint16FromHeader(uint8_t* buf, const int target);

  /**
   * @brief チェックサムを計算
   * @param data データポインタ
   * @param size データサイズ
   * @param start 開始オフセット
   * @return チェックサム値
   */
  static uint16_t calculateChecksum(const void* data, size_t size, size_t start = 0);

  /**
   * @brief シリアルパケットを作成
   * @param packet 出力先バッファ
   * @param header ヘッダー
   * @param body ボディ
   */
  static void createSerialPacket(uint8_t* packet, uint16_t* header, uint8_t* body);
};

#endif // PACKET_HANDLER_H
