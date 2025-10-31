#include "ControlledDrv8833.h"

ControlledDrv8833::ControlledDrv8833(const ControlledDrv8833Config& config)
    : Drv8833(config.in1_pin, config.in2_pin, config.pwm_frequency, config.invert_direction),
      config_(config),
      encoder_(config.enc_a_pin),
      pid_vel(config.pid_kp, config.pid_ki, config.pid_kd,
              config.pid_compute_interval_us / 1000000.0f) {  // dtを正しく設定
  if (config.enc_a_pin + 1 != config.enc_b_pin) {
    // エンコーダーピンが連続していない場合の警告
  }

  ticks_per_rev_ = config.enc_ppr * config.gear_ratio * 4.0f;
}

ControlledDrv8833::~ControlledDrv8833() {}

void ControlledDrv8833::init() {
  Drv8833::init();
  encoder_.begin();
  encoder_.reset(0);
  pid_vel.setLimit(-1.0, 1.0);  // Drv8833::run()の入力範囲に合わせる

  encoder_timer_.reset();
  pid_vel.reset();
}

void ControlledDrv8833::setTargetRpm(int16_t rpm) {
  // 目標RPMを設定
  target_rpm_ = rpm;
}

float ControlledDrv8833::runControlLoop() {
  float current_rpm = getEncoderRpm();
  float error_rpm = target_rpm_ - current_rpm;

  pid_vel.appendError(error_rpm);
  pid_vel.compute();

  float out = pid_vel.getPID();  // -1.0〜1.0の範囲
  Drv8833::run(out);
  Serial2.printf(
  "Target: %.2f, Current: %.2f, Error: %.2f, Out: %.3f (PWM: %d)\n",
  target_rpm_, current_rpm, error_rpm, out, (int)(out * 255));
  return out;
}

float ControlledDrv8833::getEncoderRps() {
  // エンコーダーカウントからRPSを計算して返す
  int32_t count = encoder_.getCount();
  count_error_ = count - last_encoder_count_;
  last_encoder_count_ = count;

  float dt_sec = encoder_timer_.read_us() / 1'000'000.0f;

  if (dt_sec > 0) {
    float rps = (count_error_ / ticks_per_rev_) / dt_sec;
    encoder_timer_.reset();
    return rps;
  }

  return 0.0f;
}

float ControlledDrv8833::getEncoderRpm() { return getEncoderRps() * 60.0f; }