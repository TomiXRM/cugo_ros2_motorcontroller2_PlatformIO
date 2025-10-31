#ifndef DRV8833_H
#define DRV8833_H

#include <Arduino.h>

class Drv8833 {
 public:
  Drv8833(uint8_t in1, uint8_t in2, uint32_t pwm_frequency);
  void init();
  void run(float speed);
  void stop();

 private:
  uint8_t in1_pin_;
  uint8_t in2_pin_;
  uint32_t pwm_frequency_;
};

#endif