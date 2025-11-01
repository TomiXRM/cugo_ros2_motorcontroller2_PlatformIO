#include "GenericMotorController.h"

GenericMotorController::GenericMotorController(DriverConfig left_config,
                                               DriverConfig right_config)
    : left_driver_(left_config), right_driver_(right_config) {}

GenericMotorController::~GenericMotorController() {}

void GenericMotorController::init() {
  left_driver_.init();
  right_driver_.init();
}

void GenericMotorController::setRPM(float left_rpm, float right_rpm) {
  left_driver_.setTargetRpm(left_rpm);
  right_driver_.setTargetRpm(right_rpm);
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
  // PID制御ループを実行
  left_driver_.runControlLoop();
  right_driver_.runControlLoop();
}

float GenericMotorController::getMaxRPM() {
  return max_rpm_;
}

float GenericMotorController::getEncoderRpmLeft() {
  return left_driver_.getEncoderRpm();
}

float GenericMotorController::getEncoderRpmRight() {
  return right_driver_.getEncoderRpm();
}
