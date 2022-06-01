#pragma once

#include <stm32f1xx.h>
#include "system.h"
#include "PID.h"

#define CH_MODE_FLOATING	0
#define CH_MODE_VOLTAGE		1
#define CH_MODE_CURRENT		2

struct Channel_TypeDef {
	Channel_TypeDef(GPIO_TypeDef *enableGPIO, uint32_t enablePIN, INA226_TypeDef *ina226) {
		this->gpio = enableGPIO;
		this->pin = enablePIN;
		this->ina226 = ina226;
	}
	void Handler(void);
	void SetVoltage(float vSet);
	void SetCurrent(float iSet);
	float GetVoltage(void);
	float GetCurrent(void);
	void SetState(uint8_t en);
	uint8_t GetState(void);
	uint8_t GetMode(void) {
		return mode;
	}
	uint8_t IsStable(void);
	
	float vSet, iSet;
	float vMeas, iMeas;
	uint8_t mode;
	PID_TypeDef pidV;
	PID_TypeDef pidI;
	float mv;
private:
	GPIO_TypeDef *gpio;
	uint32_t pin;
	INA226_TypeDef *ina226;
	uint16_t stableCounter;
};

struct OutputControl_TypeDef {
	void Init(void);
	void Handler(void);
	void SetDACValue(uint8_t ch, uint16_t val);
	void WriteDACValues(void);
	void DisableAllOutputs(void);
	Channel_TypeDef ch1 = Channel_TypeDef(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN, &ina226Ch1);
	Channel_TypeDef ch2 = Channel_TypeDef(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN, &ina226Ch2);
	Channel_TypeDef ch3 = Channel_TypeDef(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN, &ina226Ch3);
};

extern OutputControl_TypeDef outCtl;