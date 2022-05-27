#include "outputControl.h"
#include <system.h>

OutputControl_TypeDef outCtl;

uint32_t ctrlTimer;

float pv1 = 0, pv2 = 0.5, pv3 = 1.0;

// INA226 CH1 IRQ line
extern "C" void EXTI4_Handler() {
	
}

// INA226 CH2 IRQ line
extern "C" void EXTI3_Handler() {
	
}

// INA226 CH3 IRQ line
extern "C" void EXTI9_Handler() {
	
}

void OutputControl_TypeDef::Init() {
	ina226Ch1.Init();
	ina226Ch2.Init();
	ina226Ch3.Init();
}

void OutputControl_TypeDef::Handler() {
	if (system.Ticks() - ctrlTimer >= 500) {
		LL_GPIO_ResetOutputPin(AD5541_CH1_NSS_GPIO, AD5541_CH1_NSS_PIN);
		LL_SPI_TransmitData16(SPI1, 0xFFFF * pv1);
		while (LL_SPI_IsActiveFlag_BSY(SPI1)) ;
		LL_GPIO_SetOutputPin(AD5541_CH1_NSS_GPIO, AD5541_CH1_NSS_PIN);
	
		LL_GPIO_ResetOutputPin(AD5541_CH2_NSS_GPIO, AD5541_CH2_NSS_PIN);
		LL_SPI_TransmitData16(SPI1, 0xFFFF * pv2);
		while (LL_SPI_IsActiveFlag_BSY(SPI1)) ;
		LL_GPIO_SetOutputPin(AD5541_CH2_NSS_GPIO, AD5541_CH2_NSS_PIN);
	
		LL_GPIO_ResetOutputPin(AD5541_CH3_NSS_GPIO, AD5541_CH3_NSS_PIN);
		LL_SPI_TransmitData16(SPI1, 0xFFFF * pv3);
		while (LL_SPI_IsActiveFlag_BSY(SPI1)) ;
		LL_GPIO_SetOutputPin(AD5541_CH3_NSS_GPIO, AD5541_CH3_NSS_PIN);
		
		ctrlTimer = system.Ticks();
	}
}

void OutputControl_TypeDef::SetOutputState(uint8_t state) {
	if (state) {
		LL_GPIO_SetOutputPin(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN);
		LL_GPIO_SetOutputPin(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN);	
		LL_GPIO_SetOutputPin(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN);
	}
	else {
		LL_GPIO_ResetOutputPin(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN);
		LL_GPIO_ResetOutputPin(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN);
		LL_GPIO_ResetOutputPin(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN);
	}
}