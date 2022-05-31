#include <stm32f1xx.h>
#include <stm32f1xx_ll_rcc.h>
#include <stm32f1xx_ll_adc.h>
#include <stm32f1xx_ll_usart.h>
#include <stm32f1xx_ll_exti.h>
#include <stm32f1xx_ll_tim.h>
#include <stm32f1xx_ll_dma.h>

void Ticks10ms_Handler(void);
void CSSFault_Handler(void);
void ADC1_EOS_Handler(void);
void EXTI3_Handler(void);
void EXTI4_Handler(void);
void EXTI8_Handler(void);
void EXTI9_Handler(void);
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

void TIM3_IRQHandler() {
	if (LL_TIM_IsActiveFlag_CC1(TIM3)) {
		Ticks10ms_Handler();
		LL_TIM_ClearFlag_CC1(TIM3);
	}
}

void ADC1_2_IRQHandler() {
	if (LL_ADC_IsActiveFlag_EOS(ADC1)) {
		ADC1_EOS_Handler();
		LL_ADC_ClearFlag_EOS(ADC1);
	}
}

void EXTI3_IRQHandler() {
	EXTI3_Handler();
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_3);
}

void EXTI4_IRQHandler() {
	EXTI4_Handler();
	LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_4);	
}

void EXTI9_5_IRQHandler() {
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_8)) {
		EXTI8_Handler();
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_8);
	}
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_9)) {
		EXTI9_Handler();
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_9);
	}
}