#include "PID.hpp"

PID::PID(float _p, float _i, float _d, float _dt) {
    p = _p;
    i = _i;
    d = _d;
    dt = _dt;
    limit = 1000;
    setLimit(limit);
    lastError = 0;
}

void PID::setLimit(float _limit) {
    limit = abs(_limit);
    min = -limit;
    max = limit;
}

void PID::setLimit(float _limitMin, float _limitMax) {
    min = _limitMin;
    max = _limitMax;
}

void PID::setGain(float _p, float _i, float _d) {
    p = _p;
    i = _i;
    d = _d;
}

void PID::appendError(float _error) {
    error = _error;
}

void PID::reset() {
    integral = 0;
    error = 0;
    lastError = 0;
}

void PID::resetIntegral() {
    integral = 0;
}

void PID::compute() {
    if (t.read_us() >= dt * 1000000) {
        t.reset();
        // please append error before compute
        float _error = error;

        // P項とD項を計算
        pTerm = p * _error;                    // 比例
        dTerm = d * (_error - lastError) / dt; // 微分
        lastError = _error;

        // 積分を仮計算
        float proposed_integral = integral + _error * dt;
        iTerm = i * proposed_integral;

        // PID出力を計算
        _output = (pTerm + iTerm + dTerm);
        output = constrain(_output, min, max);

        // アンチワインドアップ: 出力が飽和していない場合のみ積分を更新
        if (output == _output) {
            integral = proposed_integral;
        }
        // 飽和している場合は積分を更新しない（コンディショナルインテグレーション）
    }
}
float PID::getPID() {
    return output;
}
