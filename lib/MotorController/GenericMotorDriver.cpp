#include "GenericMotorDriver.h"

#include <algorithm>
#include <cmath>

#include "DebugPrint.h"

namespace {
constexpr float kMicroSecondsToSeconds = 1.0f / 1'000'000.0f;
constexpr float kSecondsToMinutes = 60.0f;
constexpr float kIntegralGuardMin = 0.0f;
constexpr float kIntegralGuardMax = 1'000'000.0f;
}  // namespace

GenericMotorDriver::GenericMotorDriver(uint8_t pwm_pin, uint8_t dir_pin,
                                       uint8_t enc_a_pin, uint8_t enc_b_pin,
                                       float max_rpm)
    : pwm_pin_(pwm_pin),
      dir_pin_(dir_pin),
      enc_a_pin_(enc_a_pin),
      enc_b_pin_(enc_b_pin),
      max_rpm_(max_rpm) {
  recalcTicksPerRev();

  if (enc_b_pin_ != static_cast<uint8_t>(enc_a_pin_ + 1)) {
    Log.warningln(
        "GenericMotorDriver: Encoder pins must be sequential for PIO");
  }

  encoder_ = new PioEncoder(enc_a_pin_);
}

GenericMotorDriver::~GenericMotorDriver() { delete encoder_; }

void GenericMotorDriver::init() {
  pinMode(pwm_pin_, OUTPUT);
  pinMode(dir_pin_, OUTPUT);

  analogWriteFreq(pwm_frequency_);
  analogWrite(pwm_pin_, 0);
  digitalWrite(dir_pin_, LOW);

  initialize();

  last_control_us_ = micros();
  initialized_ = true;
  Log.infoln("GenericMotorDriver initialized (ticks/rev=%.2f)", ticks_per_rev_);
}

void GenericMotorDriver::initialize() {
  if (!encoder_) { return; }
  encoder_->begin();
  encoder_->reset(0);
  last_count_ = encoder_->getCount();

  target_rpm_ = 0.0f;
  measured_rpm_ = 0.0f;
  filtered_rpm_ = 0.0f;
  output_ = 0.0f;
  resetPid();
}

void GenericMotorDriver::setTargetRPM(float rpm) {
  target_rpm_ = std::clamp(rpm, -max_rpm_, max_rpm_);
}

void GenericMotorDriver::stop() {
  if (!initialized_) {
    target_rpm_ = 0.0f;
    return;
  }

  target_rpm_ = 0.0f;
  turnMotor(0.0f);
  resetPid();
}

long GenericMotorDriver::getEncoderCount() {
  return encoder_ ? encoder_->getCount() : 0;
}

void GenericMotorDriver::update() {
  if (!initialized_ || ticks_per_rev_ <= 0.0f || !encoder_) { return; }

  const uint32_t now = micros();
  if (last_control_us_ == 0) {
    last_control_us_ = now;
    last_count_ = encoder_->getCount();  // 初回はカウントも記録
    return;
  }

  const uint32_t elapsed = now - last_control_us_;
  if (elapsed < control_interval_us_) { return; }

  float dt = static_cast<float>(elapsed) * kMicroSecondsToSeconds;
  if (dt <= 0.0f) { return; }

  // 異常に長い時間が経過した場合は、この周期をスキップして次を待つ
  if (dt > 0.100f) {
    Log.warningln("GenericMotorDriver: dt too large (%.3f s), skipping cycle",
                  dt);
    last_control_us_ = now;
    last_count_ = encoder_->getCount();  // カウントもリセット
    resetPid();                          // PI状態もリセット
    return;
  }

  last_control_us_ = now;
  runControlLoop(dt);
}

float GenericMotorDriver::getMaxRPM() const { return max_rpm_; }

float GenericMotorDriver::getMeasuredRPM() const { return measured_rpm_; }

float GenericMotorDriver::getFilteredRPM() const { return filtered_rpm_; }

void GenericMotorDriver::runControlLoop(float dt_sec) {
  // 1) エンコーダ増分から現在の回転数[RPM]を算出
  const long current_count = encoder_->getCount();
  const long delta = current_count - last_count_;
  last_count_ = current_count;

  const float revs = (ticks_per_rev_ != 0.0f)
                         ? static_cast<float>(delta) / ticks_per_rev_
                         : 0.0f;
  const float rpm = revs / dt_sec * kSecondsToMinutes;
  measured_rpm_ = rpm;  // 生のRPMを記録

  // 2) 速度ローパスフィルタ（急峻な変化を抑える）
  if (!pid_.initialized) {
    filtered_rpm_ = rpm;
  } else {
    filtered_rpm_ =
        lowpass_alpha_ * rpm + (1.0f - lowpass_alpha_) * filtered_rpm_;
  }

  // 3) PI制御（シンプル版）
  const float error = target_rpm_ - filtered_rpm_;

  // 積分更新
  pid_.integral += error * dt_sec;

  // シンプルな積分ガード（±max_rpm相当）
  const float integral_limit =
      (pid_.ki != 0.0f) ? max_rpm_ / std::abs(pid_.ki) : 1000.0f;
  pid_.integral = std::clamp(pid_.integral, -integral_limit, integral_limit);

  // PI出力（max_rpmで正規化して-1.0〜+1.0の範囲に）
  float command = 0.0f;
  if (max_rpm_ != 0.0f) {
    const float pi_output = pid_.kp * error + pid_.ki * pid_.integral;
    command = pi_output / max_rpm_;
  }

  // 出力クリップ + アンチワインドアップ
  if (command > max_duty_) {
    command = max_duty_;
    pid_.integral -= error * dt_sec * 0.5f;  // 積分を戻す
  } else if (command < -max_duty_) {
    command = -max_duty_;
    pid_.integral -= error * dt_sec * 0.5f;
  }

  pid_.initialized = true;

  // 4) PWM/方向ピンに反映
  turnMotor(command);
  Serial2.printf(
      "TargetRPM: %.2f MeasuredRPM: %.2f FilteredRPM: %.2f Command: %.3f\n",
      target_rpm_, measured_rpm_, filtered_rpm_, command);
}

void GenericMotorDriver::turnMotor(float value) {
  output_ = value;

  const int pwm = abs(int(value));

  if (value < 0) {
    digitalWrite(dir_pin_, 1);
    analogWrite(pwm_pin_, 255 * pwm + 255);
  } else {
    digitalWrite(dir_pin_, 0);
    analogWrite(pwm_pin_, 255 * pwm);
  }
//   float magnitude = std::fabs(value);
//   if (magnitude < deadband_) { magnitude = 0.0f; }
//   magnitude = std::clamp(magnitude, 0.0f, max_duty_);

//   bool forward = (value >= 0.0f);
//   if (inverted_) { forward = !forward; }

//   digitalWrite(dir_pin_, forward ? LOW : HIGH);
//   const int pwm = static_cast<int>(std::round(magnitude * 255.0f));
//   analogWrite(pwm_pin_, std::clamp(pwm, 0, 255));
}

void GenericMotorDriver::resetPid() {
  pid_.integral = 0.0f;
  pid_.prev_error = 0.0f;
  pid_.initialized = false;
}

void GenericMotorDriver::recalcTicksPerRev() {
  if (counts_override_ > 0.0f) {
    ticks_per_rev_ = counts_override_;
  } else {
    ticks_per_rev_ = mechanical_ppr_ * 4.0f * gear_ratio_;
  }
}

// ---- 設定API ----

void GenericMotorDriver::setPidGains(float kp, float ki, float kd) {
  pid_.kp = kp;
  pid_.ki = ki;
  pid_.kd = kd;
  resetPid();
}

void GenericMotorDriver::setGearRatio(float ratio) {
  if (ratio <= 0.0f) { return; }
  gear_ratio_ = ratio;
  recalcTicksPerRev();
}

void GenericMotorDriver::setVelocityLowpassAlpha(float alpha) {
  lowpass_alpha_ = std::clamp(alpha, 0.0f, 1.0f);
}

void GenericMotorDriver::setMaxDuty(float duty) {
  max_duty_ = std::clamp(duty, 0.0f, 1.0f);
}

void GenericMotorDriver::setDeadband(float duty) {
  deadband_ = std::clamp(duty, 0.0f, 0.5f);
}

void GenericMotorDriver::setMaxRPM(float rpm) {
  if (rpm <= 0.0f) { return; }
  max_rpm_ = rpm;
}

void GenericMotorDriver::setFeedforwardGain(float gain) {
  feedforward_gain_ = std::max(0.0f, gain);
}

void GenericMotorDriver::setPwmFrequency(uint32_t freq_hz) {
  if (freq_hz == 0) { return; }
  pwm_frequency_ = freq_hz;
  if (initialized_) { analogWriteFreq(pwm_frequency_); }
}

void GenericMotorDriver::applyConfig(const DriverConfig& config) {
  if (config.mechanical_ppr > 0.0f) { mechanical_ppr_ = config.mechanical_ppr; }
  if (config.gear_ratio > 0.0f) { gear_ratio_ = config.gear_ratio; }
  counts_override_ =
      (config.counts_per_rev > 0.0f) ? config.counts_per_rev : 0.0f;
  control_interval_us_ =
      std::max(config.control_interval_us, static_cast<uint32_t>(200));
  inverted_ = config.invert_direction;
  recalcTicksPerRev();

  setPidGains(config.pid_kp, config.pid_ki, config.pid_kd);
  setMaxDuty(config.max_duty);
  setDeadband(config.deadband);
  if (config.max_rpm > 0.0f) { setMaxRPM(config.max_rpm); }
  setFeedforwardGain(config.feedforward_gain);
  setVelocityLowpassAlpha(config.velocity_alpha);
  setPwmFrequency(config.pwm_frequency_hz);
}
