#include "systemConfig.h"

System_TypeDef system;

extern "C" volatile uint32_t ticks;

void System_TypeDef::RCC_Init() {
	LL_UTILS_PLLInitTypeDef PLLInit_Struct;
	
	PLLInit_Struct.Prediv = LL_RCC_PREDIV_DIV_1;
	PLLInit_Struct.PLLMul = LL_RCC_PLL_MUL_9;
	
	LL_UTILS_ClkInitTypeDef ClkInit_Struct;
	
	ClkInit_Struct.AHBCLKDivider = LL_RCC_SYSCLK_DIV_1;
	ClkInit_Struct.APB1CLKDivider = LL_RCC_APB1_DIV_2;
	ClkInit_Struct.APB2CLKDivider = LL_RCC_APB2_DIV_1;
	
	LL_PLL_ConfigSystemClock_HSE(8000000U, LL_UTILS_HSEBYPASS_OFF, &PLLInit_Struct, &ClkInit_Struct);
	LL_RCC_HSE_EnableCSS();
	LL_RCC_HSI_Disable();
	
	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	LL_InitTick(SystemCoreClock, 1000);
	LL_SYSTICK_EnableIT();
}

void System_TypeDef::GPIO_Init() {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
	
	LL_GPIO_InitTypeDef GPIOInit_Struct;
	
	// Low frequency output pins
	GPIOInit_Struct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIOInit_Struct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIOInit_Struct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	
	GPIOInit_Struct.Pin = LED_PWR_PIN;
	LL_GPIO_Init(LED_PWR_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = LED_STA_PIN;
	LL_GPIO_Init(LED_STA_GPIO, &GPIOInit_Struct);
	LL_GPIO_AF_Remap_SWJ_NOJTAG();
	
	GPIOInit_Struct.Pin = LCD_BKLT_PIN;
	LL_GPIO_Init(LCD_BKLT_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = PWR_LATCH_PIN;
	LL_GPIO_Init(PWR_LATCH_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = BEEPER_PIN;
	LL_GPIO_Init(BEEPER_GPIO, &GPIOInit_Struct);
	
	// Digital input pins
	GPIOInit_Struct.Mode = LL_GPIO_MODE_INPUT;
	GPIOInit_Struct.Pull = LL_GPIO_PULL_UP;
	
	GPIOInit_Struct.Pin = BTN_PWR_PIN;
	LL_GPIO_Init(BTN_PWR_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = XPT2046_IRQ_PIN;
	LL_GPIO_Init(XPT2046_IRQ_GPIO, &GPIOInit_Struct);
}

uint32_t System_TypeDef::Ticks() {
	return ticks;
}