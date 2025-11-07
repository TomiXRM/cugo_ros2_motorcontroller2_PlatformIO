#ifndef RPM_UTILS_H
#define RPM_UTILS_H

#include "IMotorController.h"

/**
 * @brief モーターのRPM値を格納するための構造体
 */
struct MotorRPM {
  float left;
  float right;
};

/**
 * @brief RPMに関するユーティリティ関数を提供するクラス
 */
class RpmUtils {
public:
  /**
   * @brief RPMをシンプルにクランプ
   * @param target_rpm 目標RPM
   * @param max_rpm 最大RPM
   * @return クランプされたRPM
   */
  static MotorRPM clampRpmSimple(MotorRPM target_rpm, float max_rpm);

  /**
   * @brief RPMを回転優先でクランプ
   *
   * 直進成分を優先的に減速し、回転成分を残すことで
   * 曲がりきれず激突することを防止する
   *
   * @param target_rpm 目標RPM
   * @param max_rpm 最大RPM
   * @return クランプされたRPM
   */
  static MotorRPM clampRpmRotationPriority(MotorRPM target_rpm, float max_rpm);

  /**
   * @brief プロダクトIDから最大RPMを取得
   * @param product_id プロダクトID (0: V4, 1: V3i)
   * @param controller モーターコントローラー
   * @return 最大RPM
   */
  static float getMaxRpm(int product_id, IMotorController* controller);
};

#endif // RPM_UTILS_H
