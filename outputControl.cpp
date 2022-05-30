#include "outputControl.h"
#include <system.h>

OutputControl_TypeDef outCtl;

uint32_t ctrlTimer;
volatile uint8_t spiDACTransCount;
volatile uint16_t spiDACBuffer[3];

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

extern "C" void SPI1_IRQHandler() {
	if (LL_SPI_IsActiveFlag_TXE(SPI1) && !LL_SPI_IsActiveFlag_BSY(SPI1) && spiDACTransCount) {
		spiDACTransCount--;
		switch (spiDACTransCount) {
		case 2:
			LL_GPIO_SetOutputPin(AD5541_CH1_NSS_GPIO, AD5541_CH1_NSS_PIN);
			LL_GPIO_ResetOutputPin(AD5541_CH2_NSS_GPIO, AD5541_CH2_NSS_PIN);
			LL_SPI_TransmitData16(SPI1, spiDACBuffer[1]);
			break;
		case 1:
			LL_GPIO_SetOutputPin(AD5541_CH2_NSS_GPIO, AD5541_CH2_NSS_PIN);
			LL_GPIO_ResetOutputPin(AD5541_CH3_NSS_GPIO, AD5541_CH3_NSS_PIN);
			LL_SPI_TransmitData16(SPI1, spiDACBuffer[2]);
			break;
		case 0:
			LL_GPIO_SetOutputPin(AD5541_CH3_NSS_GPIO, AD5541_CH3_NSS_PIN);
			LL_SPI_DisableIT_TXE(SPI1);
			break;
		}
	}
}

extern "C" void DMA1CH7_TC_Handler() {
	ina226Ch1.DMARx_IRQ_Handler();
}
	
extern "C" void DMA1CH6_TC_Handler() {
	
}

void OutputControl_TypeDef::Init() {
	ina226Ch1.Init();
	ina226Ch2.Init();
	ina226Ch3.Init();
}

void OutputControl_TypeDef::Handler() {
	if (system.Ticks() - ctrlTimer >= 500) {
		ina226Ch1.ReadData();
		ina226Ch2.ReadData();
		ina226Ch3.ReadData();
		
		SetDACValue(1, 0xFFFF * pv1);
		SetDACValue(2, 0xFFFF * pv2);
		SetDACValue(3, 0xFFFF * pv3);
		WriteDACValues();
		
		ctrlTimer = system.Ticks();
	}
}

void OutputControl_TypeDef::SetDACValue(uint8_t ch, uint16_t val) {
	spiDACBuffer[ch - 1] = val;
}

void OutputControl_TypeDef::WriteDACValues() {
	if (spiDACTransCount == 0) {
		spiDACTransCount = 3;
		LL_GPIO_ResetOutputPin(AD5541_CH1_NSS_GPIO, AD5541_CH1_NSS_PIN);
		LL_SPI_TransmitData16(SPI1, spiDACBuffer[0]);
		LL_SPI_EnableIT_TXE(SPI1);
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

uint8_t OutputControl_TypeDef::IsOutputEnabled() {
	uint8_t t = 0;
	t |= LL_GPIO_IsOutputPinSet(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN);
	t |= LL_GPIO_IsOutputPinSet(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN);
	t |= LL_GPIO_IsOutputPinSet(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN);
	return t;
}