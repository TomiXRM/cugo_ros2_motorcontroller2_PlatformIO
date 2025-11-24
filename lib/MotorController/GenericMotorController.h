#ifndef GENERIC_MOTOR_CONTROLLER_H
#define GENERIC_MOTOR_CONTROLLER_H

#include "IMotorController.h"
#include "ControlledDrv8833.h"

/**
 * @brief 一般的なDCモータ+エンコーダを使用したモーター制御実装
 *
 * PWM制御とインクリメンタルエンコーダを使用した
 * 基本的なモーター制御のスケルトン実装
 */
class GenericMotorController : public IMotorController {
 public:
  // ドライバー設定構造体のエイリアス
  using DriverConfig = ControlledDrv8833Config;
  using Driver = ControlledDrv8833;

  GenericMotorController(DriverConfig left_config, DriverConfig right_config);
  ~GenericMotorController() override;

  void init() override;
  void setRPM(float left_rpm, float right_rpm) override;
  void stopMotor() override;
  long getEncoderCountLeft() override;
  long getEncoderCountRight() override;
  void update() override;
  float getMaxRPM() override;

  // RPM取得
  float getEncoderRpmLeft();   // 左モーター生RPM
  float getEncoderRpmRight();  // 右モーター生RPM

  //  private:
  Driver left_driver_;
  Driver right_driver_;

 private:
  const float max_rpm_ = 400.0f;
};

#endif  // GENERIC_MOTOR_CONTROLLER_H
