#include "system.h"

uint32_t tBeep;
uint8_t lTouch;

void Startup_Callback() {
	LL_mDelay(500);
	LL_GPIO_SetOutputPin(LCD_BKLT_GPIO, LCD_BKLT_PIN);
}

void Shutdown_Callback() {
	LL_GPIO_ResetOutputPin(LCD_BKLT_GPIO, LCD_BKLT_PIN);
	LL_mDelay(500);
}

int main() {
	system.Init(Startup_Callback, Shutdown_Callback);
	
	while (1) {
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
		
		system.Handler();
	}
}