#include "Drv8833.h"
#include <algorithm>

Drv8833::Drv8833(uint8_t in1, uint8_t in2, uint32_t pwm_frequency)
    : in1_pin_(in1), in2_pin_(in2), pwm_frequency_(pwm_frequency) {}

void Drv8833::init() {
  pinMode(in1_pin_, OUTPUT);
  pinMode(in2_pin_, OUTPUT);
  analogWriteFreq(pwm_frequency_);

  stop();
}

// speed: -1.0 to +1.0
void Drv8833::run(float speed) {
  speed = std::clamp(speed, -1.0f, 1.0f);

  // run with fast decay mode
  if (speed < 0) {
    analogWrite(in1_pin_, 0);
    analogWrite(in2_pin_, static_cast<int>(-speed * 255));
  } else if (speed > 0) {
    analogWrite(in1_pin_, static_cast<int>(speed * 255));
    analogWrite(in2_pin_, 0);
  } else {
    analogWrite(in2_pin_, 1);
    analogWrite(in2_pin_, 1);
  }
}

void Drv8833::stop() {
  analogWrite(in1_pin_, 0);
  analogWrite(in2_pin_, 0);
}
