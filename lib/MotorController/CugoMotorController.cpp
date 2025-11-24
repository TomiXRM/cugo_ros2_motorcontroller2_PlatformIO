#include "CugoMotorController.h"

CugoMotorController::CugoMotorController(int product_id)
  : product_id_(product_id) {
  // 最大RPMを設定
  if (product_id_ == 0) {
    max_rpm_ = CUGOV4_MAX_MOTOR_RPM;
  } else if (product_id_ == 1) {
    max_rpm_ = CUGOV3i_MAX_MOTOR_RPM;
  } else {
    max_rpm_ = CUGOV4_MAX_MOTOR_RPM;
  }
}

void CugoMotorController::init() {
  // CugoSDKの初期化を呼び出す
  cugo_init();
}

void CugoMotorController::setRPM(float left_rpm, float right_rpm) {
  // CugoSDKのRPM指令関数を呼び出す
  cugo_rpm_direct_instructions(left_rpm, right_rpm);
}

void CugoMotorController::stopMotor() {
  // モーターを停止
  cugo_rpm_direct_instructions(0.0, 0.0);
}

long CugoMotorController::getEncoderCountLeft() {
  // CugoSDKのグローバル変数から取得
  return cugo_current_count_L;
}

long CugoMotorController::getEncoderCountRight() {
  // CugoSDKのグローバル変数から取得
  return cugo_current_count_R;
}

void CugoMotorController::update() {
  // エンコーダーデータの更新
  ld2_get_cmd();
}

float CugoMotorController::getMaxRPM() {
  return max_rpm_;
}
