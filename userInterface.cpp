#include "userInterface.h"
#include "system.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSansOblique9pt7b.h"
#include "ctLogoBitmap.h"
#include "outputControl.h"
#include "curveTracer.h"

UserInterface_TypeDef ui;

char strbuff[200];

uint8_t uiUpdate = 1;
uint32_t uiTimer;
uint8_t uiUpdateIndex;
volatile uint32_t beepTimer;
uint32_t beepTime;
volatile uint8_t beepCount;
uint32_t touchTimer;
uint8_t screenIndex;

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
	SetBrightness(30);
	lcd.setFont(&FreeSans9pt7b);
	SplashScreen();
	LL_mDelay(2000);
	lcd.fillScreen(ILI9341_WHITE);
	
	Point_TypeDef min, max;
	min.x = 0;
	min.y = 0;
	min.z = 70;
	max.x = 320;
	max.y = 240;
	max.z = min.z;
	
	ts.SetCalibration(min, max);
	btn1.initButton(&lcd, 275, 55, 75, 45, ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE, "OFF", 1, 1);
	btn2.initButton(&lcd, 275, 210, 75, 45, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "RESET", 1, 1);
}

void UserInterface_TypeDef::Handler() {
	// Touch
	if (!LL_GPIO_IsInputPinSet(XPT2046_IRQ_GPIO, XPT2046_IRQ_PIN)){
		if (sys.Ticks() - touchTimer >= 50) {
			ts.StartConversion();
			
			if (ts.IsTouched()) {
				ts.GetPosition(&tsPos);
			}
			
			touchTimer = sys.Ticks();	
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
		touchTimer = sys.Ticks();
	}
	
	if (btn1.justPressed()) {
		Beep(50);
		
		if (outCtl.ch1.GetState() || outCtl.ch2.GetState() || outCtl.ch3.GetState()) {
			outCtl.ch1.SetState(0);
			outCtl.ch2.SetState(0);
			outCtl.ch3.SetState(0);
			
			btn1.setLabel("OFF");
			btn1.setColor(ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE);
		}
		else {			
			outCtl.ch1.SetState(1);
			outCtl.ch2.SetState(1);
			outCtl.ch3.SetState(1);
			
			btn1.setLabel("ON");
			btn1.setColor(ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE);
			
			lcd.fillRect(19, 40, 191, 182, ILI9341_WHITE);
			curveTracer.Start();
		}
	}
	if (btn2.justPressed()) {
		Beep(50);
	}
	if (btn2.justReleased()) {
		sys.Shutdown();
	}
	
	if (btn1.justPressed() || btn1.justReleased() || uiUpdate) {
		btn1.drawButton(btn1.isPressed());
	}
	if (btn2.justPressed() || btn2.justReleased() || uiUpdate) {
		btn2.drawButton(btn2.isPressed());
	}
	
	// Display
	if (sys.Ticks() - uiTimer >= 250 || uiUpdate) {
		uiUpdate = 0;
		screenIndex = 1;
		ScreenMenu();
		uiTimer = sys.Ticks();
	}
}

void UserInterface_TypeDef::ScreenMenu() {
	if (uiUpdateIndex == 0 || sys.IsStartup()) {
		uiUpdateIndex = 1;
			
		GFXcanvas16 canvas(320, 25);
		canvas.setFont(&FreeSans9pt7b);
		canvas.fillScreen(ILI9341_DARKCYAN);
		canvas.setCursor(5, 17);
		switch (screenIndex) {
		case 0:
			canvas.print("SysVar");
			break;
		case 1:
			canvas.print("Trace");
			break;
		}
		canvas.setCursor(267, 17);
		canvas.setTextColor(sys.OverTemperature() ? ILI9341_RED : ILI9341_WHITE);
		sprintf(strbuff, "%d.%dC\n", (uint8_t)sys.ReadDriverTemp(), (uint16_t)(sys.ReadDriverTemp() * 10) % 10);
		canvas.print(strbuff);
		lcd.drawRGBBitmap(0, 0, canvas.getBuffer(), 320, 25);
	}
	switch (screenIndex) {
	case 0:
		if (uiUpdateIndex == 1 || sys.IsStartup()) {
			uiUpdateIndex = 0;
		
			GFXcanvas16 canvas(110, 140);
			canvas.setFont(&FreeSans9pt7b);
			canvas.fillScreen(ILI9341_WHITE);
			canvas.setCursor(0, 18);
			//sprintf(strbuff, "Vin:%ldmV\n", sys.ReadVsenseVin());
			//canvas.setTextColor(sys.ReadVsenseVin() >= 23000 ? ILI9341_DARKGREEN : ILI9341_RED);
			//canvas.print(strbuff);
			canvas.setTextColor(outCtl.ch1.GetMode() == CH_MODE_VOLTAGE ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
			sprintf(strbuff, "V1: %dmV\n", (uint16_t)outCtl.ch1.GetVoltage());
			canvas.print(strbuff);
			canvas.setTextColor(outCtl.ch2.GetMode() == CH_MODE_VOLTAGE ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
			sprintf(strbuff, "V2: %dmV\n", (uint16_t)outCtl.ch2.GetVoltage());
			canvas.print(strbuff);
			canvas.setTextColor(outCtl.ch3.GetMode() == CH_MODE_VOLTAGE ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
			sprintf(strbuff, "V3: %dmV\n", (uint16_t)outCtl.ch3.GetVoltage());
			canvas.print(strbuff);
			
			canvas.setTextColor(outCtl.ch1.GetMode() == CH_MODE_CURRENT ? ILI9341_MAROON : ILI9341_DARKGREY);
			sprintf(strbuff, "I1: %d.%dmA\n", (int16_t)outCtl.ch1.GetCurrent(), (int16_t)(abs(outCtl.ch1.GetCurrent()) * 10) % 10);
			canvas.print(strbuff);
			canvas.setTextColor(outCtl.ch2.GetMode() == CH_MODE_CURRENT ? ILI9341_MAROON : ILI9341_DARKGREY);
			sprintf(strbuff, "I2: %d.%dmA\n", (int16_t)outCtl.ch2.GetCurrent(), (int16_t)(abs(outCtl.ch2.GetCurrent()) * 10) % 10);
			canvas.print(strbuff);
			canvas.setTextColor(outCtl.ch3.GetMode() == CH_MODE_CURRENT ? ILI9341_MAROON : ILI9341_DARKGREY);
			sprintf(strbuff, "I3: %d.%dmA\n", (int16_t)outCtl.ch3.GetCurrent(), (int16_t)(abs(outCtl.ch3.GetCurrent()) * 10) % 10);
			canvas.print(strbuff);
			lcd.drawRGBBitmap(5, 25, canvas.getBuffer(), 110, 140);
		}
		break;
	case 1:
		if (uiUpdateIndex == 1 || sys.IsStartup()) {
			uiUpdateIndex = 0;
			
			lcd.drawLine(19, 221, 210, 221, ILI9341_DARKGREY);
			lcd.drawLine(19, 221, 19, 40, ILI9341_DARKGREY);
		}
		break;
	}
}

void UserInterface_TypeDef::SplashScreen() {
	{
		lcd.drawRGBBitmap(130, 72, (uint16_t*)ctLogoBitmap, ctLogoBitmapWidth, ctLogoBitmapHeight);
		GFXcanvas16 canvas(150, 60);
		canvas.fillScreen(ILI9341_WHITE);
		canvas.setTextColor(ILI9341_DARKGREY);
		canvas.setCursor(10, 17);
		canvas.setFont(&FreeSansOblique9pt7b);
		canvas.print("SCT-2001");
		canvas.setCursor(0, 41);
		canvas.setFont(&FreeSans9pt7b);
		canvas.print("CurveTracer");
		lcd.drawRGBBitmap(103, 110, canvas.getBuffer(), 150, 60);
	}
	{
		GFXcanvas16 canvas(250, 25);
		canvas.setFont(&FreeSans9pt7b);
		canvas.fillScreen(ILI9341_WHITE);
		canvas.setTextColor(ILI9341_DARKGREY);
		canvas.setCursor(0, 17);
		sprintf(strbuff, "v%d.%d%c (%s)", FW_VER_MAJOR, FW_VER_MINOR, FW_VER_REV, FW_VER_DATE);
		canvas.print(strbuff);
		lcd.drawRGBBitmap(67, 213, canvas.getBuffer(), 250, 25);
	}
}

void UserInterface_TypeDef::Ticks10ms_IRQ_Handler() {
	// Beeper
	if (beepCount) {
		if (sys.Ticks() - beepTimer >= beepTime / (LL_GPIO_IsOutputPinSet(BEEPER_GPIO, BEEPER_PIN) ? 1 : 2)) {
			if (!LL_GPIO_IsOutputPinSet(BEEPER_GPIO, BEEPER_PIN)) {
				LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
			}
			else {
				LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
				beepCount--;
			}
			beepTimer = sys.Ticks();
		}
	}
}

void UserInterface_TypeDef::Beep(uint32_t t, uint8_t cnt) {
	if (beepCount == 0) {
		beepTime = t;
		beepCount = cnt;
		LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
		beepTimer = sys.Ticks();
	}
}

void UserInterface_TypeDef::SetBrightness(uint8_t bright) {
	LL_TIM_OC_SetCompareCH3(LCD_BKLT_TIM, bright);	
}