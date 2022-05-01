#include "systemConfig.h"

uint32_t tLed, tBeep;
uint8_t lTouch;

int main() {
	system.RCC_Init();
	system.GPIO_Init();
standby:
	LL_GPIO_ResetOutputPin(LED_STA_GPIO, LED_STA_PIN);
	while (LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) ;
	LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
	LL_mDelay(50);
	LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
	
	LL_GPIO_SetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
	LL_mDelay(100);
	LL_GPIO_SetOutputPin(LED_PWR_GPIO, LED_PWR_PIN);
	LL_mDelay(500);
	LL_GPIO_SetOutputPin(LCD_BKLT_GPIO, LCD_BKLT_PIN);
	
	while (1) {
		if (system.Ticks() - tLed >= 250) {
			LL_GPIO_TogglePin(LED_STA_GPIO, LED_STA_PIN);
			
			tLed = system.Ticks();
		}	
		if (!LL_GPIO_IsInputPinSet(XPT2046_IRQ_GPIO, XPT2046_IRQ_PIN) && !lTouch) {
			tBeep = system.Ticks();
			lTouch = 1;
			LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
		}
		else if (LL_GPIO_IsInputPinSet(XPT2046_IRQ_GPIO, XPT2046_IRQ_PIN) && lTouch) {
			lTouch = 0;
		}
		if (system.Ticks() - tBeep >= 100) {
			LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
			tBeep = system.Ticks() + 1;
		}
		if (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) {
			LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
			LL_mDelay(50);
			LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
			LL_GPIO_ResetOutputPin(LCD_BKLT_GPIO, LCD_BKLT_PIN);
			LL_mDelay(500);
			LL_GPIO_ResetOutputPin(LED_PWR_GPIO, LED_PWR_PIN);
			while (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) ;
			LL_GPIO_ResetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
			goto standby;
		}
	}
}