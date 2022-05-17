#pragma once

#include "stm32f1xx.h"

struct Serial_TypeDef {
	void Init(void);
	void Handler(void);
};

extern Serial_TypeDef serial;