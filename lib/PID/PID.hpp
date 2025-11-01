#ifndef PID_H
#define PID_H
#include <Arduino.h>
#include <timer.h>

class PID {
  private:
    /* data */
    float p, i, d; // PID gain
    float dt;      // interval of PID loop [s]

    float lastError;
    float error;

    float output, _output;
    float limit;
    float min, max;

    float pTerm;
    float iTerm, integral;
    float dTerm;
    TimerUtil t;

  public:
    PID(float _p, float _i, float _d, float _dt);
    void setLimit(float limit);
    void setLimit(float _limitMin, float _limitMax);
    void setGain(float _p, float _i, float _d);
    void reset();
    void resetIntegral();
    void appendError(float _error);
    void compute();
    float getPID();
};

#endif