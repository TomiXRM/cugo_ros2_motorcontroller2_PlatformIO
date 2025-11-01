#include "timer.h"
TimerUtil::TimerUtil(){
	tm = millis();
	tu = micros();
}

void TimerUtil::reset(){
	tm = millis();
	tu = micros();
}

unsigned long TimerUtil::read_ms(){
	return (millis() - tm);
}

unsigned long TimerUtil::read_us(){
	return (micros() - tu);
}
