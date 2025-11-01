#include "RpmUtils.h"
#include <Arduino.h>

// CugoSDK定義（USE_CUGO_SDK時のみ）
#ifdef USE_CUGO_SDK
  #define CUGOV4_MAX_MOTOR_RPM  130
  #define CUGOV3i_MAX_MOTOR_RPM 180
#endif

MotorRPM RpmUtils::clampRpmSimple(MotorRPM target_rpm, float max_rpm) {
  MotorRPM new_rpm = target_rpm;

  // 左モータ速度を監視。上限を超えてたら上限速度に丸める。
  if (abs(target_rpm.left) >= max_rpm) {
    new_rpm.left = target_rpm.left / abs(target_rpm.left) * max_rpm;
  }

  // 右モータ速度を監視。上限を超えてたら上限速度に丸める。
  if (abs(target_rpm.right) >= max_rpm) {
    new_rpm.right = target_rpm.right / abs(target_rpm.right) * max_rpm;
  }

  return new_rpm;
}

MotorRPM RpmUtils::clampRpmRotationPriority(MotorRPM target_rpm, float max_rpm) {
  // L/Rモータ速度を監視して、上限速度を超えると上限速度にクランプするコードを使用していた
  // すると、直進しながら急旋回する動きで、ロボットを回転させる速度がクランプロジックで消えてしまうことがあった
  // これにより、回避指示をしているのに障害物に激突することがあった
  // ここでは、L/Rのrpmを直進成分と回転成分を抜き出し、直進だけ減速させ回転成分を殺さないように速度上限クランプを行う処理をおこなう。

  // --- ステップ1: 目標RPMを並進速度(v_trans)と角速度(v_rot)に分解 ---
  float v_trans = (target_rpm.right + target_rpm.left) / 2.0f;
  float v_rot = (target_rpm.right - target_rpm.left) / 2.0f;

  // --- ステップ2: 角速度(v_rot)自体の上限処理 ---
  // そもそも角速度が速すぎると、並進速度を0にしても片輪が上限を超えてしまう。（並進、回転両方上限以上のとき）
  // そのため、最初にv_rotを[-max_rpm, max_rpm]の範囲に丸める。
  float clamped_v_rot = max(-max_rpm, min(max_rpm, v_rot));

  // --- ステップ3: 回転(clamped_v_rot)を維持するために、並進(v_trans)の上限を計算 ---
  // ここでは、上限処理済みの `clamped_v_rot` を使用する。
  float v_trans_limit = max_rpm - std::abs(clamped_v_rot);

  // --- ステップ4: 並進速度(v_trans)を上限値(v_trans_limit)に丸める ---
  // v_trans_limitは常に0以上になるため、よりシンプルなclamp処理が可能。
  float clamped_v_trans = max(-v_trans_limit, min(v_trans_limit, v_trans));

  // --- ステップ5: 丸めたv_transとv_rotから、最終的なRPMを再計算 ---
  // ここでも、上限処理済みの `clamped_v_rot` を使用する。
  MotorRPM new_rpm;
  new_rpm.left = clamped_v_trans - clamped_v_rot;
  new_rpm.right = clamped_v_trans + clamped_v_rot;

  return new_rpm;
}

float RpmUtils::getMaxRpm(int product_id, IMotorController* controller) {
  if (controller) {
    return controller->getMaxRPM();
  }

#ifdef USE_CUGO_SDK
  if (product_id == 0) {
    return CUGOV4_MAX_MOTOR_RPM;
  } else if (product_id == 1) {
    return CUGOV3i_MAX_MOTOR_RPM;
  } else {
    return CUGOV4_MAX_MOTOR_RPM;
  }
#else
  return 100.0;  // デフォルト値
#endif
}
