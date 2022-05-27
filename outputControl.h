#pragma once

#include <stm32f1xx.h>

struct OutputControl_TypeDef {
	void Init(void);
	void Handler(void);
	void SetOutputState(uint8_t state);
	uint8_t IsOutputEnabled();
};

extern OutputControl_TypeDef outCtl;