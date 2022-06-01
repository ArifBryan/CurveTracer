#include "PID.h"

void PID_TypeDef::Calculate(float sp, float av) {
	e = sp - av;
	float de = e - le;
	
	u = (kp * e) + (ki * (se / td)) + (kd * (de * td));
	if ((u <= outMin && e > 0) || (u >= outMax && e < 0) || (u > outMin && u < outMax)) {
		se += e;
	}
	le = e;
	
	u = (u < outMin ? outMin : (u > outMax ? outMax : u));
}

float PID_TypeDef::GetOutput() {
	return u;
}

void PID_TypeDef::Reset() {
	e = 0;
	le = 0;
	se = 0;
	u = 0;
}