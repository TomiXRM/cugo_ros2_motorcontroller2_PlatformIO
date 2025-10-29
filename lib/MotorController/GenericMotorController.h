#ifndef GENERIC_MOTOR_CONTROLLER_H
#define GENERIC_MOTOR_CONTROLLER_H

#include "IMotorController.h"
#include <Arduino.h>

/**
 * @brief 一般的なDCモータ+エンコーダを使用したモーター制御実装
 *
 * PWM制御とインクリメンタルエンコーダを使用した
 * 基本的なモーター制御のスケルトン実装
 */
class GenericMotorController : public IMotorController {
public:
  /**
   * @brief コンストラクタ
   * @param left_pwm_pin 左モーターPWMピン
   * @param left_dir_pin 左モーター方向ピン
   * @param left_enc_a_pin 左エンコーダーAピン
   * @param left_enc_b_pin 左エンコーダーBピン
   * @param right_pwm_pin 右モーターPWMピン
   * @param right_dir_pin 右モーター方向ピン
   * @param right_enc_a_pin 右エンコーダーAピン
   * @param right_enc_b_pin 右エンコーダーBピン
   * @param max_rpm 最大RPM
   */
  GenericMotorController(
    uint8_t left_pwm_pin,
    uint8_t left_dir_pin,
    uint8_t left_enc_a_pin,
    uint8_t left_enc_b_pin,
    uint8_t right_pwm_pin,
    uint8_t right_dir_pin,
    uint8_t right_enc_a_pin,
    uint8_t right_enc_b_pin,
    float max_rpm = 100.0
  );

  void init() override;
  void setRPM(float left_rpm, float right_rpm) override;
  void stopMotor() override;
  long getEncoderCountLeft() override;
  long getEncoderCountRight() override;
  void update() override;
  float getMaxRPM() override;

  // エンコーダー割り込みハンドラ（staticメンバー関数）
  static void encoderISR_LeftA();
  static void encoderISR_LeftB();
  static void encoderISR_RightA();
  static void encoderISR_RightB();

private:
  // ピン設定
  uint8_t left_pwm_pin_;
  uint8_t left_dir_pin_;
  uint8_t left_enc_a_pin_;
  uint8_t left_enc_b_pin_;
  uint8_t right_pwm_pin_;
  uint8_t right_dir_pin_;
  uint8_t right_enc_a_pin_;
  uint8_t right_enc_b_pin_;

  // 最大RPM
  float max_rpm_;

  // 目標RPM
  float target_left_rpm_;
  float target_right_rpm_;

  // エンコーダーカウント（volatile: 割り込みで更新）
  static volatile long encoder_count_left_;
  static volatile long encoder_count_right_;

  // PID制御用変数（将来実装）
  float kp_;  // 比例ゲイン
  float ki_;  // 積分ゲイン
  float kd_;  // 微分ゲイン

  // 内部ヘルパー関数
  void setPWM(uint8_t pwm_pin, uint8_t dir_pin, float rpm);
  void updatePID();  // PID制御（将来実装）
};

#endif // GENERIC_MOTOR_CONTROLLER_H
