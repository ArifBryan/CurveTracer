#pragma once

struct PID_TypeDef {
	void SetConstants(float kp, float ki, float kd, float td) {
		this->kp = kp;
		this->ki = ki;
		this->kd = kd;
		this->td = td;
	}
	void SetOutputRange(float min, float max) {
		outMin = min;
		outMax = max;
	}
	void Calculate(float sp, float av);
	float GetError(void) {
		return e;
	}
	float GetOutput(void);
	void Reset(void);
private:
	float kp, ki, kd, td;
	float e, le, se;
	float outMin, outMax;
	float u;
};