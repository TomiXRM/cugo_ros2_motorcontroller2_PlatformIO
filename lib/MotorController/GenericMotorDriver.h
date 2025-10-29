#ifndef GENERIC_MOTOR_DRIVER_H
#define GENERIC_MOTOR_DRIVER_H

#include <Arduino.h>
#include <pio_encoder.h>

/**
 * @brief 汎用DCモータを制御するためのドライバクラス。
 *
 * 2輪（左/右）構成を前提とし、RPMベースのPI(PID)制御と
 * PIOエンコーダ読み取りを行う。
 */
class GenericMotorDriver {
 public:
  struct DriverConfig {
    float mechanical_ppr{13.0f};
    float gear_ratio{45.0f};
    float counts_per_rev{0.0f};
    uint32_t control_interval_us{1000};
    bool invert_direction{false};
    float pid_kp{2.0f};
    float pid_ki{4.0f};
    float pid_kd{0.0f};
    float max_duty{1.0f};
    float deadband{0.02f};
    float max_rpm{100.0f};
    float feedforward_gain{1.0f};
    float velocity_alpha{0.3f};
    uint32_t pwm_frequency_hz{100000};
  };

  GenericMotorDriver(uint8_t left_pwm_pin, uint8_t left_dir_pin,
                     uint8_t left_enc_a_pin, uint8_t left_enc_b_pin,
                     float max_rpm = 100.0f);
  ~GenericMotorDriver();

  void init();
  void setTargetRPM(float rpm);
  void stop();
  long getEncoderCount();
  void update();
  float getMaxRPM() const;

  // RPM取得
  float getMeasuredRPM() const;     // 生のRPM（フィルタ前）
  float getFilteredRPM() const;     // フィルタ済みRPM

  // ---- 設定API ----
  void setPidGains(float kp, float ki, float kd = 0.0f);
  void setGearRatio(float ratio);
  void setVelocityLowpassAlpha(float alpha);
  void setMaxDuty(float duty);
  void setDeadband(float duty);
  void setMaxRPM(float rpm);
  void setFeedforwardGain(float gain);
  void setPwmFrequency(uint32_t freq_hz);
  void applyConfig(const DriverConfig& config);

 private:
  struct PidState {
    float kp{6.0f};
    float ki{2.0f};
    float kd{0.0f};
    float integral{0.0f};
    float prev_error{0.0f};
    bool initialized{false};
  };

  void initialize();
  void runControlLoop(float dt_sec);
  void turnMotor(float value);
  void resetPid();
  void recalcTicksPerRev();

  uint8_t pwm_pin_{0};
  uint8_t dir_pin_{0};
  uint8_t enc_a_pin_{0};
  uint8_t enc_b_pin_{0};
  PioEncoder* encoder_{nullptr};
  bool inverted_{false};

  long last_count_{0};
  float target_rpm_{0.0f};
  float measured_rpm_{0.0f};
  float filtered_rpm_{0.0f};

  float output_{0.0f};
  PidState pid_{};

  float mechanical_ppr_{13.0f};
  float gear_ratio_{45.0f};
  float counts_override_{0.0f};
  float ticks_per_rev_{0.0f};

  float max_rpm_{100.0f};
  float max_duty_{1.0f};
  float deadband_{0.02f};
  float lowpass_alpha_{0.3f};
  float feedforward_gain_{1.0f};

  uint32_t pwm_frequency_{100000};
  uint32_t control_interval_us_{1000};
  uint32_t last_control_us_{0};

  bool initialized_{false};
};

#endif  // GENERIC_MOTOR_DRIVER_H
