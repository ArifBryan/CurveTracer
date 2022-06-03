#pragma once

#include <stm32f1xx.h>

struct CurveTracer_TypeDef {
	void Init(void);
	void Handler(void);
	void Start(void);
	void Stop(void);
	uint8_t IsSampling(void);
	uint8_t IsSamplingDone(uint8_t clear);
	uint8_t IsSamplingDone() {return IsSamplingDone(0);}
private:
	uint8_t run;
};

extern CurveTracer_TypeDef curveTracer;