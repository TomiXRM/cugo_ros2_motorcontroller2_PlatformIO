#include <Servo.h>
#include <PacketSerial.h>
#include "DebugPrint.h"
#include "IMotorController.h"
#include "CugoMotorController.h"
#include "PacketHandler.h"
#include "RpmUtils.h"
// #include "GenericMotorController.h"  // 一般的なDCモータ使用時にコメント解除

// モーターコントローラーの選択
// USE_CUGO_SDK: CugoSDKを使用
// USE_GENERIC_MOTOR: 一般的なDCモータ+エンコーダを使用
// #define USE_CUGO_SDK
#define USE_GENERIC_MOTOR

uint8_t packetBinaryBufferSerial[SERIAL_HEADER_SIZE + SERIAL_BIN_BUFF_SIZE];
PacketSerial packetSerial;

// モーターコントローラーのインスタンス
IMotorController* motorController = nullptr;

unsigned long long current_time = 0, prev_time_10ms = 0, prev_time_100ms,
                   prev_time_1000ms;

// FAIL SAFE COUNT
int COM_FAIL_COUNT = 0;

// ========================================
// フェイルセーフ・モーター制御
// ========================================

void stop_motor_immediately() {
  if (motorController) {
    motorController->stopMotor();
    Log.warningln("Motor stopped (failsafe triggered)");
  }
}

void check_failsafe() {
  // 100msごとに通信の有無を確認
  // 5回連続(0.5秒)ROSからの通信が来なかった場合、直ちにロボットを停止する
  COM_FAIL_COUNT++;
  if (COM_FAIL_COUNT > 5) { stop_motor_immediately(); }
}

// ========================================
// 周期タスク
// ========================================

void job_10ms() {
  // nothing
}

void job_100ms() {
  check_failsafe();
  // エンコーダカウントを更新
  if (motorController) { motorController->update(); }
}

void job_1000ms() {
  // nothing
}

// ========================================
// モーター制御
// ========================================

void set_motor_cmd_binary(uint8_t* reciev_buf, int size, float max_rpm) {
  if (size > 0 && motorController) {
    MotorRPM reciev_rpm, clamped_rpm;
    reciev_rpm.left =
        PacketHandler::readFloatFromBuf(reciev_buf, TARGET_RPM_L_PTR);
    reciev_rpm.right =
        PacketHandler::readFloatFromBuf(reciev_buf, TARGET_RPM_R_PTR);

    // 物理的最高速以上のときは、モータの最高速に丸める
    bool rotation_clamp_logic =
        true;  // 回転成分を優先した丸めアルゴリズムを有効化
    if (rotation_clamp_logic) {
      // 回転成分を優先して残し、直進方向を減らす方法で速度上限以上の速度を丸める
      clamped_rpm = RpmUtils::clampRpmRotationPriority(reciev_rpm, max_rpm);
    } else {
      clamped_rpm = RpmUtils::clampRpmSimple(reciev_rpm, max_rpm);
    }

    motorController->setRPM(clamped_rpm.left, clamped_rpm.right);
    Log.trace("RPM set: L=%.2f R=%.2f\n", clamped_rpm.left, clamped_rpm.right);

    // モータに指令値を無事セットできたら、通信失敗カウンタをリセット
    COM_FAIL_COUNT = 0;
  } else {
    if (motorController) { motorController->stopMotor(); }
    Log.errorln("Motor command not set: invalid size or motorController is null");
  }
}

// ========================================
// パケット受信処理
// ========================================

void onSerialPacketReceived(const uint8_t* buffer, size_t size) {
  uint8_t tempBuffer[size];
  memcpy(tempBuffer, buffer, size);

  // バッファにたまったデータを抜き出して制御に適用
  uint16_t product_id = PacketHandler::readUint16FromHeader(
      tempBuffer, RECV_HEADER_PRODUCT_ID_PTR);
  float max_rpm = RpmUtils::getMaxRpm(product_id, motorController);

  // チェックサムの確認
  uint16_t recv_checksum =
      PacketHandler::readUint16FromHeader(tempBuffer, RECV_HEADER_CHECKSUM_PTR);
  const uint8_t* body_ptr = tempBuffer + SERIAL_HEADER_SIZE;
  uint16_t calc_checksum =
      PacketHandler::calculateChecksum(body_ptr, SERIAL_BIN_BUFF_SIZE);

  if (recv_checksum != calc_checksum) {
    // パケット整合性チェック失敗
    Log.errorln("Checksum mismatch (recv: 0x%04X, calc: 0x%04X)", recv_checksum, calc_checksum);
  } else {
    set_motor_cmd_binary(tempBuffer, size, max_rpm);
    Log.traceln("Motor command processed");
  }

  // 送信ボディの作成
  uint8_t send_body[SERIAL_BIN_BUFF_SIZE];
  memset(send_body, 0, sizeof(send_body));

  // ボディへ送信データの書き込み
  if (motorController) {
    long encoder_L = motorController->getEncoderCountLeft();
    long encoder_R = motorController->getEncoderCountRight();
    PacketHandler::writeIntToBuf(send_body, SEND_ENCODER_L_PTR, encoder_L);
    PacketHandler::writeIntToBuf(send_body, SEND_ENCODER_R_PTR, encoder_R);
  }

  // チェックサムの計算
  uint16_t checksum =
      PacketHandler::calculateChecksum(send_body, SERIAL_BIN_BUFF_SIZE);
  uint16_t send_len = SERIAL_HEADER_SIZE + SERIAL_BIN_BUFF_SIZE;

  // 送信ヘッダの作成
  uint16_t localPort = 8888;
  uint16_t send_header[4] = {localPort, 8888, send_len, checksum};

  // 送信パケットの作成
  uint8_t send_packet[send_len];
  PacketHandler::createSerialPacket(send_packet, send_header, send_body);
  packetSerial.send(send_packet, send_len);
}

// ========================================
// setup / loop
// ========================================

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  // デバッグシリアル初期化
  debugInit();
  Log.noticeln("=== Cugo Motor Controller Started ===");

#ifdef USE_CUGO_SDK
  Log.infoln("Using CugoSDK");
  // CugoSDKを使用
  motorController = new CugoMotorController(0);  // 0: V4, 1: V3i
#else
  Log.infoln("Using Generic Motor Controller");
  // 一般的なDCモータ+エンコーダを使用
  // ピン番号は実際のハードウェアに合わせて変更してください
  // motorController = new GenericMotorController(
  //   2,  // left_pwm_pin
  //   3,  // left_dir_pin
  //   4,  // left_enc_a_pin
  //   5,  // left_enc_b_pin
  //   6,  // right_pwm_pin
  //   7,  // right_dir_pin
  //   8,  // right_enc_a_pin
  //   9,  // right_enc_b_pin
  //   100.0  // max_rpm
  // );
#endif

  if (motorController) {
    motorController->init();
    Log.infoln("Motor controller initialized");
  }

  // PacketSerial初期化
  packetSerial.begin(115200);
  packetSerial.setStream(&Serial);
  packetSerial.setPacketHandler(&onSerialPacketReceived);
  Log.infoln("PacketSerial initialized");

  // Serialバッファをカラにしてから実行を開始する
  delay(100);
  while (Serial.available() > 0) { Serial.read(); }

  Log.noticeln("Setup completed");
}

void loop() {
  current_time = micros();

  if (current_time - prev_time_10ms > 10000) {
    job_10ms();
    prev_time_10ms = current_time;
  }

  if (current_time - prev_time_100ms > 100000) {
    job_100ms();
    prev_time_100ms = current_time;
  }

  if (current_time - prev_time_1000ms > 1000000) {
    job_1000ms();
    prev_time_1000ms = current_time;
  }

  // シリアル通信でコマンドを受信
  packetSerial.update();

  // 受信バッファのオーバーフローチェック
  if (packetSerial.overflow()) { Log.errorln("PacketSerial overflow!"); }
}
