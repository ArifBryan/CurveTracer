#include "stm32f1xx.h"
#include "stm32f1xx_ll_rcc.h"

volatile uint32_t ticks;
void SysTick_Handler() {
	ticks++;
}

void CSSFault_Handler(void) {}

void NMI_Handler(void) {
	if (LL_RCC_IsActiveFlag_HSECSS()) {
		LL_RCC_ClearFlag_HSECSS();
		CSSFault_Handler();
	}
}