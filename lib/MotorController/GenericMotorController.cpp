#include "GenericMotorController.h"
#include "DebugPrint.h"

// static変数の定義
volatile long GenericMotorController::encoder_count_left_ = 0;
volatile long GenericMotorController::encoder_count_right_ = 0;

// グローバルポインタ（割り込みハンドラから参照するため）
static GenericMotorController* g_instance = nullptr;

GenericMotorController::GenericMotorController(
  uint8_t left_pwm_pin,
  uint8_t left_dir_pin,
  uint8_t left_enc_a_pin,
  uint8_t left_enc_b_pin,
  uint8_t right_pwm_pin,
  uint8_t right_dir_pin,
  uint8_t right_enc_a_pin,
  uint8_t right_enc_b_pin,
  float max_rpm
)
  : left_pwm_pin_(left_pwm_pin),
    left_dir_pin_(left_dir_pin),
    left_enc_a_pin_(left_enc_a_pin),
    left_enc_b_pin_(left_enc_b_pin),
    right_pwm_pin_(right_pwm_pin),
    right_dir_pin_(right_dir_pin),
    right_enc_a_pin_(right_enc_a_pin),
    right_enc_b_pin_(right_enc_b_pin),
    max_rpm_(max_rpm),
    target_left_rpm_(0.0),
    target_right_rpm_(0.0),
    kp_(0.1),
    ki_(0.0),
    kd_(0.0) {

  g_instance = this;
}

void GenericMotorController::init() {
  DEBUG_PRINTLN("GenericMotorController::init()");

  // PWMピンの初期化
  pinMode(left_pwm_pin_, OUTPUT);
  pinMode(left_dir_pin_, OUTPUT);
  pinMode(right_pwm_pin_, OUTPUT);
  pinMode(right_dir_pin_, OUTPUT);

  // エンコーダーピンの初期化
  pinMode(left_enc_a_pin_, INPUT_PULLUP);
  pinMode(left_enc_b_pin_, INPUT_PULLUP);
  pinMode(right_enc_a_pin_, INPUT_PULLUP);
  pinMode(right_enc_b_pin_, INPUT_PULLUP);

  // エンコーダー割り込みの設定
  attachInterrupt(digitalPinToInterrupt(left_enc_a_pin_), encoderISR_LeftA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(left_enc_b_pin_), encoderISR_LeftB, CHANGE);
  attachInterrupt(digitalPinToInterrupt(right_enc_a_pin_), encoderISR_RightA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(right_enc_b_pin_), encoderISR_RightB, CHANGE);

  // エンコーダーカウントのリセット
  encoder_count_left_ = 0;
  encoder_count_right_ = 0;

  DEBUG_PRINTLN("GenericMotorController initialized");
}

void GenericMotorController::setRPM(float left_rpm, float right_rpm) {
  target_left_rpm_ = left_rpm;
  target_right_rpm_ = right_rpm;

  // TODO: PID制御実装後は、update()で周期的に制御
  // 現在はシンプルなオープンループ制御
  setPWM(left_pwm_pin_, left_dir_pin_, left_rpm);
  setPWM(right_pwm_pin_, right_dir_pin_, right_rpm);
}

void GenericMotorController::stopMotor() {
  target_left_rpm_ = 0.0;
  target_right_rpm_ = 0.0;
  setPWM(left_pwm_pin_, left_dir_pin_, 0.0);
  setPWM(right_pwm_pin_, right_dir_pin_, 0.0);
}

long GenericMotorController::getEncoderCountLeft() {
  return encoder_count_left_;
}

long GenericMotorController::getEncoderCountRight() {
  return encoder_count_right_;
}

void GenericMotorController::update() {
  // TODO: PID制御によるフィードバック制御を実装
  // 現在の実装：エンコーダーから速度を計算し、PID制御でPWMを調整
  updatePID();
}

float GenericMotorController::getMaxRPM() {
  return max_rpm_;
}

void GenericMotorController::setPWM(uint8_t pwm_pin, uint8_t dir_pin, float rpm) {
  // RPMをPWM値に変換（0-255）
  // TODO: 実際のモーターに合わせて調整が必要
  float duty_ratio = abs(rpm) / max_rpm_;
  uint16_t pwm_value = constrain(duty_ratio * 255, 0, 255);

  // 方向設定
  if (rpm >= 0) {
    digitalWrite(dir_pin, HIGH);
  } else {
    digitalWrite(dir_pin, LOW);
  }

  // PWM出力
  analogWrite(pwm_pin, pwm_value);
}

void GenericMotorController::updatePID() {
  // TODO: PID制御を実装
  // 1. エンコーダーから現在のRPMを計算
  // 2. 目標RPMと現在RPMの誤差を計算
  // 3. PID演算でPWM値を調整
  // 現在は何もしない（setRPMで直接PWMを設定）
}

// エンコーダー割り込みハンドラ
void GenericMotorController::encoderISR_LeftA() {
  // TODO: エンコーダーのA/B相を読み取って回転方向を判定
  // 簡易実装：A相の立ち上がり/立ち下がりでカウント
  if (g_instance) {
    bool a = digitalRead(g_instance->left_enc_a_pin_);
    bool b = digitalRead(g_instance->left_enc_b_pin_);

    // A相とB相の位相差から回転方向を判定
    if (a == b) {
      encoder_count_left_++;
    } else {
      encoder_count_left_--;
    }
  }
}

void GenericMotorController::encoderISR_LeftB() {
  // B相の変化でもカウント（分解能2倍）
  if (g_instance) {
    bool a = digitalRead(g_instance->left_enc_a_pin_);
    bool b = digitalRead(g_instance->left_enc_b_pin_);

    if (a != b) {
      encoder_count_left_++;
    } else {
      encoder_count_left_--;
    }
  }
}

void GenericMotorController::encoderISR_RightA() {
  if (g_instance) {
    bool a = digitalRead(g_instance->right_enc_a_pin_);
    bool b = digitalRead(g_instance->right_enc_b_pin_);

    if (a == b) {
      encoder_count_right_++;
    } else {
      encoder_count_right_--;
    }
  }
}

void GenericMotorController::encoderISR_RightB() {
  if (g_instance) {
    bool a = digitalRead(g_instance->right_enc_a_pin_);
    bool b = digitalRead(g_instance->right_enc_b_pin_);

    if (a != b) {
      encoder_count_right_++;
    } else {
      encoder_count_right_--;
    }
  }
}
