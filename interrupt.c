#include <stm32f1xx.h>
#include <stm32f1xx_ll_rcc.h>
#include <stm32f1xx_ll_adc.h>

void CSSFault_Handler(void);
void ADC1_EOS_Handler(void);

volatile uint32_t ticks;
void SysTick_Handler() {
	ticks++;
}

void NMI_Handler() {
	if (LL_RCC_IsActiveFlag_HSECSS()) {
		CSSFault_Handler();
		LL_RCC_ClearFlag_HSECSS();
	}
}

void ADC1_2_IRQHandler() {
	if (LL_ADC_IsActiveFlag_EOS(ADC1)) {
		ADC1_EOS_Handler();
		LL_ADC_ClearFlag_EOS(ADC1);
	}
}