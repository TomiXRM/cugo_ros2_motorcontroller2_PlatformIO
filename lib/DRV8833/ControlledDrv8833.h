#ifndef CONTROLLED_DRV8833_H
#define CONTROLLED_DRV8833_H

#include <Arduino.h>
#include "Drv8833.h"
#include "PID.hpp"
#include "pio_encoder.h"
#include "timer.h"

struct ControlledDrv8833Config {
  uint8_t in1_pin;    // PWM pin
  uint8_t in2_pin;    // PWM pin
  uint8_t enc_a_pin;  // Encoder A pin
  uint8_t enc_b_pin;  // Encoder B pin
  uint32_t pwm_frequency{10000};
  bool invert_direction{true};

  float enc_ppr{13};
  float gear_ratio{45.0f};

  uint16_t pid_compute_interval_us{1000};
  float pid_kp{0.0001f};
  float pid_ki{0.0f};
  float pid_kd{0.0f};
};

class ControlledDrv8833 : public Drv8833 {
 public:
  ControlledDrv8833(const ControlledDrv8833Config& config);
  ~ControlledDrv8833();

  void init();

  void setTargetRpm(int16_t rpm);

  float runControlLoop();

  float getEncoderRps();

  float getEncoderRpm();

  void resetPid() { pid_vel.reset(); }

 private:
  ControlledDrv8833Config config_;
  PID pid_vel;
  PioEncoder encoder_;
  TimerUtil encoder_timer_;

  int32_t last_encoder_count_{0};
  int32_t count_error_{0};
  float ticks_per_rev_{0.0f};
  float target_rpm_{0.0f};

  float max_rpm_{300.0f};
  float min_rpm_{-300.0f};
};

#endif