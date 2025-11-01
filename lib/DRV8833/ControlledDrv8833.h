#ifndef CONTROLLED_DRV8833_H
#define CONTROLLED_DRV8833_H

#include <Arduino.h>
#include "Drv8833.h"
#include "PID.hpp"
#include "pio_encoder.h"
#include "timer.h"

typedef struct {
  uint8_t in1_pin;    // PWM pin
  uint8_t in2_pin;    // PWM pin
  uint8_t enc_a_pin;  // Encoder A pin
  uint8_t enc_b_pin;  // Encoder B pin
  uint32_t pwm_frequency{10000};
  bool invert_direction{true};

  float enc_ppr{13.0f};  // エンコーダーのパルス数 (1回転あたり)
  float gear_ratio{45.0f};
  float wheel_diameter_mm{65.0f};  // ホイール直径 [mm]
  float desired_distance_per_count_mm{0.4f};  // 上位コンピュータが期待する1カウントあたりの移動量 [mm]

  uint16_t pid_compute_interval_us{1000};
  float pid_kp{0.0001f};
  float pid_ki{0.0f};
  float pid_kd{0.0f};
} ControlledDrv8833Config;

class ControlledDrv8833 : public Drv8833 {
 public:
  ControlledDrv8833(const ControlledDrv8833Config& config);
  ~ControlledDrv8833();

  void init();

  void setTargetRpm(float rpm);

  float runControlLoop();

  float getEncoderRps();

  float getEncoderRpm();

  int32_t getEncoderCount();

  int32_t getEncoderIncrement();

  void resetPid() { pid_vel.reset(); }

 private:
  ControlledDrv8833Config config_;
  PID pid_vel;
  PioEncoder encoder_;
  TimerUtil encoder_timer_;

  int32_t last_encoder_count_{0};
  int32_t count_error_{0};
  float pulse_per_shaft_rev_{0.0f};
  float target_rpm_{0.0f};
  float encoder_scale_factor_{1.0f};  // エンコーダカウントのスケーリング係数

  float max_rpm_{300.0f};
  float min_rpm_{-300.0f};
};

#endif