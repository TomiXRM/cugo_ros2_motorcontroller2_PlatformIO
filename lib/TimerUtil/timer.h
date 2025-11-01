#ifndef TIMER_H
#define TIMER_H

#include "Arduino.h"

class TimerUtil {
public:
	TimerUtil();
	void reset();
	unsigned long read_ms();
	unsigned long read_us();
    unsigned long tm;
    unsigned long tu;
private:
};
#endif