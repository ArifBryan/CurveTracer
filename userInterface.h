#pragma once

#include <stm32f1xx.h>

struct UserInterface_TypeDef {
	void Init(void);
	void Handler(void);
	void SetBrightness(uint8_t bright);
	void Beep(uint32_t t, uint8_t cnt = 1);
	uint8_t IsTouched(void);
	void GetTouchPosition(uint16_t *x, uint16_t *y);
	void Ticks10ms_IRQ_Handler();
};

extern UserInterface_TypeDef ui;