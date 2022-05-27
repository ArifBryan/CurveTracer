#include "userInterface.h"
#include "system.h"

UserInterface_TypeDef ui;

GFXcanvas16 canvas(120, 120);
char strbuff[200];

uint32_t uiTimer;
uint32_t beepTimer;
uint32_t beepTime;
uint8_t beepCount;

volatile uint8_t touch;

// XPT2046 IRQ line
extern "C" void EXTI8_Handler() {
	touch = 1;
	
}

void UserInterface_TypeDef::Init() {
	ts.Init();
	ts.SetRotation(1);
	lcd.Init();
	lcd.setRotation(1);
	lcd.fillScreen(ILI9341_BLACK);
	LL_mDelay(10);
	SetBrightness(40);
	canvas.setFont(&FreeSans9pt7b);
	canvas.setTextColor(ILI9341_WHITE);
	canvas.setTextSize(1);
	canvas.setCursor(0, 18);
	canvas.print("SysVal");
	lcd.drawRGBBitmap(0, 0, canvas.getBuffer(), 120, 120);
}

Point_TypeDef tsPos;

void UserInterface_TypeDef::Handler() {
	// Touch
	if (touch) {
		ts.StartConversion();
		ts.GetPosition(&tsPos);
		
		Beep(50);
			
		LL_GPIO_TogglePin(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN);
		LL_GPIO_TogglePin(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN);
		LL_GPIO_TogglePin(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN);
		
		touch = 0;
	}
	
	// Display
	if (system.Ticks() - uiTimer >= 500) {		
		float v1 = ina226Ch1.Read(INA226_VBUS) * 1.25;
		float v2 = ina226Ch2.Read(INA226_VBUS) * 1.25;
		float v3 = ina226Ch3.Read(INA226_VBUS) * 1.25;
		
		canvas.fillScreen(ILI9341_BLACK);
		canvas.setCursor(0, 18);
		sprintf(strbuff, "Vin: %ldmV  \n", system.ReadVsenseVin());
		canvas.setTextColor(system.ReadVsenseVin() >= 23000 ? ILI9341_GREEN : ILI9341_RED);
		canvas.print(strbuff);
		canvas.setTextColor(system.ReadDriverTemp() >= 33 ? (system.OverTemperature() ? ILI9341_RED : ILI9341_ORANGE) : ILI9341_GREEN);
		sprintf(strbuff, "T. : %d.%02dC\n", (uint8_t)system.ReadDriverTemp(), (uint16_t)(system.ReadDriverTemp() * 100) % 100);
		canvas.print(strbuff);
		canvas.setTextColor(LL_GPIO_IsOutputPinSet(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN) ? ILI9341_GREEN : ILI9341_DARKGREY);
		sprintf(strbuff, "V1: %dmV\n", (uint16_t)v1);
		canvas.print(strbuff);
		canvas.setTextColor(LL_GPIO_IsOutputPinSet(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN) ? ILI9341_GREEN : ILI9341_DARKGREY);
		sprintf(strbuff, "V2: %dmV\n", (uint16_t)v2);
		canvas.print(strbuff);
		canvas.setTextColor(LL_GPIO_IsOutputPinSet(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN) ? ILI9341_GREEN : ILI9341_DARKGREY);
		sprintf(strbuff, "V3: %dmV\n", (uint16_t)v3);
		canvas.print(strbuff);
		lcd.drawRGBBitmap(0, 25, canvas.getBuffer(), 120, 120);
		
		canvas.fillScreen(ILI9341_BLACK);
		canvas.setCursor(0, 18);
		canvas.setTextColor(ILI9341_WHITE);
		sprintf(strbuff, "x: %d\ny: %d\nz: %d", tsPos.x, tsPos.y, tsPos.z);
		canvas.print(strbuff);
		lcd.drawRGBBitmap(0, 145, canvas.getBuffer(), 120, 120);
		
		uiTimer = system.Ticks();
	}
	
	// Beeper
	if (beepCount) {
		if (system.Ticks() - beepTimer >= beepTime / (LL_GPIO_IsOutputPinSet(BEEPER_GPIO, BEEPER_PIN) ? 1 : 2)) {
			if (!LL_GPIO_IsOutputPinSet(BEEPER_GPIO, BEEPER_PIN)) {
				LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
			}
			else {
				LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
				beepCount--;
			}
			beepTimer = system.Ticks();
		}
	}
}

void UserInterface_TypeDef::Beep(uint32_t t, uint8_t cnt) {
	if (beepCount == 0) {
		beepTime = t;
		beepCount = cnt;
		LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
		beepTimer = system.Ticks();
	}
}

void UserInterface_TypeDef::SetBrightness(uint8_t bright) {
	LL_TIM_OC_SetCompareCH3(LCD_BKLT_TIM, bright);	
}