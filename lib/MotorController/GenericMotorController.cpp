#include "GenericMotorController.h"

GenericMotorController::GenericMotorController(
    uint8_t left_pwm_pin, uint8_t left_dir_pin, uint8_t left_enc_a_pin,
    uint8_t left_enc_b_pin, uint8_t right_pwm_pin, uint8_t right_dir_pin,
    uint8_t right_enc_a_pin, uint8_t right_enc_b_pin, float max_rpm)
    : left_driver_(left_pwm_pin, left_dir_pin, left_enc_a_pin, left_enc_b_pin,
                   max_rpm),
      right_driver_(right_pwm_pin, right_dir_pin, right_enc_a_pin,
                    right_enc_b_pin, max_rpm) {}

GenericMotorController::~GenericMotorController() = default;

void GenericMotorController::init() {
  left_driver_.init();
  right_driver_.init();
}

void GenericMotorController::setRPM(float left_rpm, float right_rpm) {
  left_driver_.setTargetRPM(left_rpm);
  right_driver_.setTargetRPM(right_rpm);
}

void GenericMotorController::stopMotor() {
  left_driver_.stop();
  right_driver_.stop();
}

long GenericMotorController::getEncoderCountLeft() {
  return left_driver_.getEncoderCount();
}

long GenericMotorController::getEncoderCountRight() {
  return right_driver_.getEncoderCount();
}

void GenericMotorController::update() {
  left_driver_.update();
  right_driver_.update();
}

float GenericMotorController::getMaxRPM() { return left_driver_.getMaxRPM(); }

float GenericMotorController::getMeasuredRPMLeft() {
  return left_driver_.getMeasuredRPM();
}

float GenericMotorController::getMeasuredRPMRight() {
  return right_driver_.getMeasuredRPM();
}

float GenericMotorController::getFilteredRPMLeft() {
  return left_driver_.getFilteredRPM();
}

float GenericMotorController::getFilteredRPMRight() {
  return right_driver_.getFilteredRPM();
}

void GenericMotorController::applyDriverConfigs(
    const DriverConfig& left_config, const DriverConfig& right_config) {
  left_driver_.applyConfig(left_config);
  right_driver_.applyConfig(right_config);
}

void GenericMotorController::applyDriverConfigSymmetric(
    const DriverConfig& config) {
  applyDriverConfigs(config, config);
}
