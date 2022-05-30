#include "userInterface.h"
#include "system.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSansOblique9pt7b.h"
#include "ctLogoBitmap.h"
#include "outputControl.h"

UserInterface_TypeDef ui;

char strbuff[200];

uint32_t uiTimer;
uint8_t uiUpdateIndex;
volatile uint32_t beepTimer;
uint32_t beepTime;
volatile uint8_t beepCount;
uint32_t touchTimer;

Point_TypeDef tsPos;

Adafruit_GFX_Button btn1;
Adafruit_GFX_Button btn2;

extern "C" void EXTI8_Handler() {
	
}

void UserInterface_TypeDef::Init() {
	ts.Init();
	ts.SetRotation(1);
	lcd.Init();
	lcd.setRotation(1);
	lcd.fillScreen(ILI9341_WHITE);
	LL_mDelay(10);
	SetBrightness(25);
	lcd.setFont(&FreeSans9pt7b);
	{
		lcd.drawRGBBitmap(130, 72, (uint16_t*)ctLogoBitmap, ctLogoBitmapWidth, ctLogoBitmapHeight);
		GFXcanvas16 canvas(150, 60);
		canvas.fillScreen(ILI9341_WHITE);
		canvas.setTextColor(ILI9341_DARKGREY);
		canvas.setTextSize(1);
		canvas.setCursor(10, 17);
		canvas.setFont(&FreeSansOblique9pt7b);
		canvas.print("SCT-2001");
		canvas.setCursor(0, 40);
		canvas.setFont(&FreeSans9pt7b);
		canvas.print("CurveTracer");
		lcd.drawRGBBitmap(103, 110, canvas.getBuffer(), 150, 60);
		LL_mDelay(2000);
		lcd.fillScreen(ILI9341_WHITE);
	}
	{
		GFXcanvas16 canvas(320, 25);
		canvas.setFont(&FreeSans9pt7b);
		canvas.fillScreen(ILI9341_DARKGREEN);
		canvas.setTextColor(ILI9341_WHITE);
		canvas.setTextSize(1);
		canvas.setCursor(5, 17);
		canvas.print("SysVal");
		lcd.drawRGBBitmap(0, 0, canvas.getBuffer(), 320, 25);
	}
	
	Point_TypeDef min, max;
	min.x = 0;
	min.y = 0;
	min.z = 70;
	max.x = 320;
	max.y = 240;
	max.z = min.z;
	
	ts.SetCalibration(min, max);
	btn1.initButton(&lcd, 270, 60, 75, 45, ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE, "OFF", 1, 1);
	btn2.initButton(&lcd, 270, 110, 75, 45, ILI9341_DARKGREY, ILI9341_DARKCYAN, ILI9341_WHITE, "OFF", 1, 1);
	btn1.drawButton();
	btn2.drawButton();
}

void UserInterface_TypeDef::Handler() {
	// Touch
	if (!LL_GPIO_IsInputPinSet(XPT2046_IRQ_GPIO, XPT2046_IRQ_PIN)){
		if (system.Ticks() - touchTimer >= 50) {
			ts.StartConversion();
			
			if (ts.IsTouched()) {
				ts.GetPosition(&tsPos);
			}
			
			touchTimer = system.Ticks();	
		}
		if (btn1.contains(tsPos.x, tsPos.y)) {
			btn1.press(1);
		}
		else if (btn2.contains(tsPos.x, tsPos.y)) {
			btn2.press(1);
		}
	}
	else {
		btn1.press(0);
		btn2.press(0);
		tsPos.x = 0;
		tsPos.y = 0;
		tsPos.z = 0;
		touchTimer = system.Ticks();
	}
	
	if (btn1.justPressed()) {
		Beep(50);
		if (outCtl.IsOutputEnabled()) {
			outCtl.SetOutputState(0);
			btn1.setLabel("OFF");
			btn1.setColor(ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE);
		}
		else {			
			outCtl.SetOutputState(1);
			btn1.setLabel("ON");
			btn1.setColor(ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE);
		}
	}
	
	if (btn1.justPressed() || btn1.justReleased()) {
		btn1.drawButton(btn1.isPressed());
	}
	if (btn2.justPressed() || btn2.justReleased()) {
		btn2.drawButton(btn2.isPressed());
	}
	
	// Display
	if (system.Ticks() - uiTimer >= 250) {
		if (uiUpdateIndex == 0) {
			uiUpdateIndex = 1;
			float v1 = ina226Ch1.GetVoltage();
			float v2 = ina226Ch2.GetVoltage();
			float v3 = ina226Ch3.GetVoltage();
		
			GFXcanvas16 canvas(120, 120);
			canvas.setFont(&FreeSans9pt7b);
			canvas.fillScreen(ILI9341_WHITE);
			canvas.setCursor(0, 18);
			sprintf(strbuff, "Vin:%ldmV\n", system.ReadVsenseVin());
			canvas.setTextColor(system.ReadVsenseVin() >= 23000 ? ILI9341_DARKGREEN : ILI9341_RED);
			canvas.print(strbuff);
			canvas.setTextColor(LL_GPIO_IsOutputPinSet(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN) ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
			sprintf(strbuff, "V1: %dmV\n", (uint16_t)v1);
			canvas.print(strbuff);
			canvas.setTextColor(LL_GPIO_IsOutputPinSet(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN) ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
			sprintf(strbuff, "V2: %dmV\n", (uint16_t)v2);
			canvas.print(strbuff);
			canvas.setTextColor(LL_GPIO_IsOutputPinSet(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN) ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
			sprintf(strbuff, "V3: %dmV\n", (uint16_t)v3);
			canvas.print(strbuff);
			lcd.drawRGBBitmap(0, 25, canvas.getBuffer(), 120, 120);
		}
		else if (uiUpdateIndex == 1) {
			uiUpdateIndex = 0;
			
			GFXcanvas16 canvas(100, 25);
			canvas.setFont(&FreeSans9pt7b);
			canvas.fillScreen(ILI9341_DARKGREEN);
			canvas.setCursor(35, 17);
			canvas.setTextColor(system.ReadDriverTemp() >= 31 ? (system.OverTemperature() ? ILI9341_RED : ILI9341_ORANGE) : ILI9341_WHITE);
			sprintf(strbuff, "%d.%02dC\n", (uint8_t)system.ReadDriverTemp(), (uint16_t)(system.ReadDriverTemp() * 100) % 100);
			canvas.print(strbuff);
			lcd.drawRGBBitmap(220, 0, canvas.getBuffer(), 100, 25);
		}
		
		uiTimer = system.Ticks();
	}
}

void UserInterface_TypeDef::Ticks10ms_IRQ_Handler() {
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