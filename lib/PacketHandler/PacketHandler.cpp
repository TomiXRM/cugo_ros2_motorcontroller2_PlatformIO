#include "PacketHandler.h"

void PacketHandler::writeFloatToBuf(uint8_t* buf, const int target, float val) {
  uint8_t* val_ptr = reinterpret_cast<uint8_t*>(&val);
  memmove(buf + target, val_ptr, sizeof(float));
}

void PacketHandler::writeIntToBuf(uint8_t* buf, const int target, int val) {
  uint8_t* val_ptr = reinterpret_cast<uint8_t*>(&val);
  memmove(buf + target, val_ptr, sizeof(int));
}

void PacketHandler::writeBoolToBuf(uint8_t* buf, const int target, bool val) {
  uint8_t* val_ptr = reinterpret_cast<uint8_t*>(&val);
  memmove(buf + target, val_ptr, sizeof(bool));
}

float PacketHandler::readFloatFromBuf(uint8_t* buf, const int target) {
  float val = *reinterpret_cast<float*>(buf + SERIAL_HEADER_SIZE + target);
  return val;
}

int PacketHandler::readIntFromBuf(uint8_t* buf, const int target) {
  int val = *reinterpret_cast<int*>(buf + SERIAL_HEADER_SIZE + target);
  return val;
}

bool PacketHandler::readBoolFromBuf(uint8_t* buf, const int target) {
  bool val = *reinterpret_cast<bool*>(buf + SERIAL_HEADER_SIZE + target);
  return val;
}

uint8_t PacketHandler::readUint8FromBuf(uint8_t* buf, const int target) {
  uint8_t val = *reinterpret_cast<uint8_t*>(buf + SERIAL_HEADER_SIZE + target);
  return val;
}

uint16_t PacketHandler::readUint16FromHeader(uint8_t* buf, const int target) {
  if (target >= SERIAL_HEADER_SIZE - 1) return 0;
  uint16_t val = *reinterpret_cast<uint16_t*>(buf + target);
  return val;
}

uint16_t PacketHandler::calculateChecksum(const void* data, size_t size, size_t start) {
  uint32_t sum = 0;  // 桁あふれを考慮して32bitで計算
  const uint8_t* bytes = static_cast<const uint8_t*>(data);

  for (size_t i = start; i < size; i += 2) {
    // リトルエンディアンのバイト列から正しく16bit整数を復元
    // (上位バイト << 8) | 下位バイト
    uint16_t word = (static_cast<uint16_t>(bytes[i + 1]) << 8) | static_cast<uint16_t>(bytes[i]);
    sum += word;
  }

  // 桁あふれ処理 (キャリーを回収)
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }

  // 1の補数 (ビット反転)
  return static_cast<uint16_t>(~sum);
}

void PacketHandler::createSerialPacket(uint8_t* packet, uint16_t* header, uint8_t* body) {
  size_t offset = 0;
  memmove(packet, header, sizeof(uint8_t) * SERIAL_HEADER_SIZE);
  offset += sizeof(uint8_t) * SERIAL_HEADER_SIZE;
  memmove(packet + offset, body, sizeof(uint8_t) * SERIAL_BIN_BUFF_SIZE);
}
