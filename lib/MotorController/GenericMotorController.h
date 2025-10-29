#ifndef GENERIC_MOTOR_CONTROLLER_H
#define GENERIC_MOTOR_CONTROLLER_H

#include "IMotorController.h"
#include "GenericMotorDriver.h"

/**
 * @brief 一般的なDCモータ+エンコーダを使用したモーター制御実装
 *
 * PWM制御とインクリメンタルエンコーダを使用した
 * 基本的なモーター制御のスケルトン実装
 */
class GenericMotorController : public IMotorController {
 public:
  using DriverConfig = GenericMotorDriver::DriverConfig;

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
  GenericMotorController(uint8_t left_pwm_pin, uint8_t left_dir_pin,
                         uint8_t left_enc_a_pin, uint8_t left_enc_b_pin,
                         uint8_t right_pwm_pin, uint8_t right_dir_pin,
                         uint8_t right_enc_a_pin, uint8_t right_enc_b_pin,
                         float max_rpm = 100.0);
  ~GenericMotorController() override;

  void init() override;
  void setRPM(float left_rpm, float right_rpm) override;
  void stopMotor() override;
  long getEncoderCountLeft() override;
  long getEncoderCountRight() override;
  void update() override;
  float getMaxRPM() override;

  // RPM取得
  float getMeasuredRPMLeft();    // 左モーター生RPM
  float getMeasuredRPMRight();   // 右モーター生RPM
  float getFilteredRPMLeft();    // 左モーターフィルタ済みRPM
  float getFilteredRPMRight();   // 右モーターフィルタ済みRPM

  // ---- 設定API ----
  void applyDriverConfigs(const DriverConfig& left_config,
                          const DriverConfig& right_config);
  void applyDriverConfigSymmetric(const DriverConfig& config);

//  private:
  GenericMotorDriver left_driver_;
  GenericMotorDriver right_driver_;
};

#endif  // GENERIC_MOTOR_CONTROLLER_H
