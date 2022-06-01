#pragma once

#include <stm32f1xx.h>

struct CurveTracer_TypeDef {
	void Init(void);
	void Handler(void);
	void Start(void);
private:
	uint8_t run;
};

extern CurveTracer_TypeDef curveTracer;