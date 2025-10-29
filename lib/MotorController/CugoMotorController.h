#ifndef CUGO_MOTOR_CONTROLLER_H
#define CUGO_MOTOR_CONTROLLER_H

#include "IMotorController.h"
#include "CugoSDK.h"

/**
 * @brief CugoSDKを使用したモーター制御実装
 *
 * 既存のCugoSDKをラップして、IMotorControllerインターフェイスに適合させる
 */
class CugoMotorController : public IMotorController {
public:
  /**
   * @brief コンストラクタ
   * @param product_id プロダクトID (0: V4, 1: V3i)
   */
  CugoMotorController(int product_id = 0);

  void init() override;
  void setRPM(float left_rpm, float right_rpm) override;
  void stopMotor() override;
  long getEncoderCountLeft() override;
  long getEncoderCountRight() override;
  void update() override;
  float getMaxRPM() override;

private:
  int product_id_;
  float max_rpm_;
};

#endif // CUGO_MOTOR_CONTROLLER_H
