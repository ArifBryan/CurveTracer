#pragma once

#include <stm32f1xx.h>

struct OutputControl_TypeDef {
	void Init(void);
	void Handler(void);
	void SetOutputState(uint8_t state);
	uint8_t IsOutputEnabled();
	void SetDACValue(uint8_t ch, uint16_t val);
	void WriteDACValues(void);
};

extern OutputControl_TypeDef outCtl;