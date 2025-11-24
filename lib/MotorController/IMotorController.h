#ifndef I_MOTOR_CONTROLLER_H
#define I_MOTOR_CONTROLLER_H

/**
 * @brief モーター制御の抽象インターフェイス
 *
 * CugoSDKと一般的なDCモータ+エンコーダを
 * 切り替えられるようにするための基底クラス
 */
class IMotorController {
public:
  virtual ~IMotorController() = default;

  /**
   * @brief 初期化処理
   */
  virtual void init() = 0;

  /**
   * @brief モーターRPM指令を設定
   * @param left_rpm 左モーターの目標RPM
   * @param right_rpm 右モーターの目標RPM
   */
  virtual void setRPM(float left_rpm, float right_rpm) = 0;

  /**
   * @brief モーターを即座に停止
   */
  virtual void stopMotor() = 0;

  /**
   * @brief 左エンコーダーの累積カウントを取得
   * @return 左エンコーダーカウント
   */
  virtual long getEncoderCountLeft() = 0;

  /**
   * @brief 右エンコーダーの累積カウントを取得
   * @return 右エンコーダーカウント
   */
  virtual long getEncoderCountRight() = 0;

  /**
   * @brief 周期更新処理（エンコーダー読み取りなど）
   *
   * main loopの100ms周期タスクなどから呼び出す
   */
  virtual void update() = 0;

  /**
   * @brief モーターの最大RPMを取得
   * @return 最大RPM
   */
  virtual float getMaxRPM() = 0;
};

#endif // I_MOTOR_CONTROLLER_H
