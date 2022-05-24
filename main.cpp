#include "system.h"
#include "serial.h"

#include "ILI9341.h"

uint32_t tBeep;
uint8_t lTouch;

uint32_t vin, vs;
float temp;
uint8_t bklt = 50;

void Startup_Handler() {
	serial.Init();
	lcd.Init();
	lcd.setRotation(1);
	lcd.fillScreen(ILI9341_BLACK);
	LL_mDelay(10);
	LL_TIM_OC_SetCompareCH3(LCD_BKLT_TIM, bklt);
	ina226Ch1.Init();
	ina226Ch2.Init();
	ina226Ch3.Init();
}

void Shutdown_Handler() {
	LL_TIM_OC_SetCompareCH3(LCD_BKLT_TIM, 0);
	LL_mDelay(500);
}

void OverTemperature_Handler() {
	system.Shutdown();
}

LL_RCC_ClocksTypeDef clk;

int main() {
	system.Init(Startup_Handler, Shutdown_Handler, OverTemperature_Handler);
	lcd.setFont(&FreeSans9pt7b);
	lcd.setTextColor(ILI9341_WHITE);
	lcd.setTextSize(1);
	lcd.setCursor(0, 18);
	lcd.print("SysVal");
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
		
		LL_TIM_OC_SetCompareCH3(LCD_BKLT_TIM, bklt);
		
		LL_RCC_GetSystemClocksFreq(&clk);
		
		system.Handler();
		serial.Handler();
		
		vin = system.ReadVsenseVin();
		vs = system.ReadVsense5V();
		temp = system.ReadDriverTemp();
	}
}