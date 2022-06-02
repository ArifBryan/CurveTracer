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

TouchOverlay_TypeDef tb1;
TouchOverlay_TypeDef tb2;
TouchOverlay_TypeDef tb3;

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
	keypad.Init();
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
		
		if (keypad.IsEnabled()) {
			keypad.TouchHandler(tsPos, 1);			
		}
		else {
			btn1.press(btn1.contains(tsPos.x, tsPos.y));
			btn2.press(btn2.contains(tsPos.x, tsPos.y));
			tb1.TouchHandler(tsPos, 1);
			tb2.TouchHandler(tsPos, 1);
			tb3.TouchHandler(tsPos, 1);
		}
	}
	else {
		keypad.TouchHandler(tsPos, 0);
		btn1.press(0);
		btn2.press(0);
		tb1.TouchHandler(tsPos, 0);
		tb2.TouchHandler(tsPos, 0);
		tb3.TouchHandler(tsPos, 0);
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
		
	// Button refresh
	ButtonHandler();
	keypad.Handler();
	if (keypad.IsPressed()) {
		Beep(50);
		if (keypad.GetKey() == 'O') {
			uiUpdate = 1;
		}
		else if (keypad.GetKey() == 'C') {
			uiUpdate = 1;
		}
	}
	if (!keypad.IsEnabled()) {
		if (btn1.justPressed() || btn1.justReleased() || uiUpdate) {
			btn1.drawButton(btn1.isPressed());
		}
		if (btn2.justPressed() || btn2.justReleased() || uiUpdate) {
			btn2.drawButton(btn2.isPressed());
		}
	}
	
	// Display
	if (sys.Ticks() - uiTimer >= 250 || uiUpdate) {
		uiUpdate = 0;
		// Force screen menu
		screenIndex = 0;
		ScreenMenu();
		uiTimer = sys.Ticks();
	}
}


void UserInterface_TypeDef::SetScreenMenu(uint8_t index) {
	screenIndex = index;
	uiUpdateIndex = 0;
	uiUpdate = 1;
}

void UserInterface_TypeDef::ButtonHandler() {
	switch (screenIndex) {
	case 0:
		if (btn2.justPressed()) {
			Beep(50);
		}
		if (btn2.justReleased()) {
			sys.Shutdown();
		}
		
		if (tb1.JustPressed()) {
			Beep(50);
		}
		if (tb2.JustPressed()) {
			Beep(50);
		}
		if (tb3.JustPressed()) {
			Beep(50);
		}
		break;
	case 1:
		if (btn2.justPressed()) {
			Beep(50);
		}
		if (btn2.justReleased()) {
			sys.Shutdown();
		}
		break;
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
			canvas.print("Source & Measure");
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
	if (!keypad.IsEnabled()) {
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
				tb1.Init(canvas.getCursorX() + 5, canvas.getCursorY() + 25, 100, 10);
				canvas.setTextColor(outCtl.ch1.GetMode() == CH_MODE_VOLTAGE ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
				sprintf(strbuff, "V1: %dmV\n", (uint16_t)outCtl.ch1.GetVoltage());
				canvas.print(strbuff);
				tb2.Init(canvas.getCursorX() + 5, canvas.getCursorY() + 25, 100, 10);
				canvas.setTextColor(outCtl.ch2.GetMode() == CH_MODE_VOLTAGE ? ILI9341_DARKGREEN : ILI9341_DARKGREY);
				sprintf(strbuff, "V2: %dmV\n", (uint16_t)outCtl.ch2.GetVoltage());
				canvas.print(strbuff);
				tb3.Init(canvas.getCursorX() + 5, canvas.getCursorY() + 25, 100, 10);
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
}

void TouchOverlay_TypeDef::Init(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
}

void TouchOverlay_TypeDef::TouchHandler(Point_TypeDef pos, bool touch) {
	int16_t px = pos.x;
	int16_t py = pos.y;
	
	stateLast = stateNow;
	if (((px >= x) && (px < (int16_t)(x + w)) && (py >= y) && (py < (int16_t)(y + h))) && touch) {
		stateNow = 1;
	}
	else {stateNow = 0;}
}

void Keypad_TypeDef::Handler() {
	pressedKey = 0;
	if (enable) {
		for (uint8_t i = 0; i < 16; i++) {
			if (key[i].justPressed()) {
				pressedKey = i + 1;
			}
			if (key[i].justPressed() || key[i].justReleased() || refresh) {
				key[i].drawButton(key[i].isPressed());
			}
		}
		if (IsPressed() || refresh) {
			refresh = 1;
			if (IsPressed()) {
				if (keyLUT[pressedKey - 1] == 'O' || keyLUT[pressedKey - 1] == 'C') {
					Disable();
					refresh = 0;
				}
				else if (keyLUT[pressedKey - 1] == 'A') {
					if (keyBufferPtr > 0) {
						decPoint = 0;
						keyBufferPtr = 0;
						memset(keyBuffer, 0, 19);
					}
				}
				else if (keyLUT[pressedKey - 1] == 'D') {
					if (keyBufferPtr > 0) {
						if (keyBufferPtr == decPoint) {decPoint = 0; }
						keyBuffer[--keyBufferPtr] = 0;
					}
				}
				else if (keyBufferPtr < 18) {
					if (!(keyBufferPtr > 0 && keyLUT[pressedKey - 1] == '-') && !(keyLUT[pressedKey - 1] == '.' && decPoint > 0)) {
						if (keyLUT[pressedKey - 1] == '.') {decPoint = keyBufferPtr + 1; }
						keyBuffer[keyBufferPtr++] = keyLUT[pressedKey - 1];				
					}
					else {
						pressedKey = 0;
					}
				}
				else {
					pressedKey = 0;
				}
			}
			
			if (refresh) {
				GFXcanvas16 textBox(190, 25);
			
				textBox.fillScreen(ILI9341_WHITE);
				textBox.setTextColor(ILI9341_DARKGREY);
				textBox.setFont(&FreeSans9pt7b);
				textBox.drawRoundRect(0, 0, 190, 25, 1, ILI9341_DARKCYAN);
				textBox.setCursor(4, 18);
				textBox.print(keyBuffer);
				textBox.setCursor(textBox.getCursorX(), 16);
				textBox.print("|");
				lcd.drawRGBBitmap(30, 30, textBox.getBuffer(), 190, 25);
			}
		}
		refresh = 0;
	}
}

void Keypad_TypeDef::TouchHandler(Point_TypeDef pos, uint8_t touch) {
	for (uint8_t i = 0; i < 16; i++) {
		key[i].press(key[i].contains(pos.x, pos.y) && touch);
	}
}

float Keypad_TypeDef::GetKeyFloat() {
	float t;
	sscanf(keyBuffer, "%f", &t);
	return t;
}
int Keypad_TypeDef::GetKeyInteger() {
	int t;
	sscanf(keyBuffer, "%d", &t);
	return t;
}

uint8_t Keypad_TypeDef::IsPressed() {
	return pressedKey > 0;
}

void Keypad_TypeDef::Enable() {
	if (!enable) {
		enable = 1;
		refresh = 1;
		decPoint = 0;
		keyBufferPtr = 0;
		memset(keyBuffer, 0, 19);
		lcd.fillRect(30, 30, 390, 210, ILI9341_WHITE);
	}
}
void Keypad_TypeDef::Disable() {
	if (enable) {
		enable = 0;
		lcd.fillRect(30, 30, 390, 210, ILI9341_WHITE);
	}
}
uint8_t Keypad_TypeDef::IsEnabled() {
	return enable;
}

void Keypad_TypeDef::Init() {
	uint16_t xPos = 30, yPos = 60;
	uint16_t x = xPos, y = yPos;
	uint16_t btnWidth = 60;
	uint16_t btnHeight = 40;
	uint16_t btnPad = 5;
	
	key[0].initButtonUL(&lcd, x, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "1", 1, 1);
	key[1].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "2", 1, 1);
	key[2].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "3", 1, 1);
	key[3].initButtonUL(&lcd, 255, y, btnWidth, btnHeight, ILI9341_ORANGE, ILI9341_ORANGE, ILI9341_WHITE, "<", 1, 1);
	x = xPos;
	y += btnHeight + btnPad;
	key[4].initButtonUL(&lcd, x, y, 60, 40, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "4", 1, 1);
	key[5].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "5", 1, 1);
	key[6].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "6", 1, 1);
	key[7].initButtonUL(&lcd, 255, y, btnWidth, btnHeight, ILI9341_ORANGE, ILI9341_ORANGE, ILI9341_WHITE, "C", 1, 1);
	x = xPos;
	y += btnHeight + btnPad;
	key[8].initButtonUL(&lcd, x, y, 60, 40, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "7", 1, 1);
	key[9].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "8", 1, 1);
	key[10].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "9", 1, 1);
	key[11].initButtonUL(&lcd, 255, y, btnWidth, btnHeight, ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE, "OK", 1, 1);
	x = xPos;
	y += btnHeight + btnPad;
	key[12].initButtonUL(&lcd, x, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, ".", 1, 1);
	key[13].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "0", 1, 1);
	key[14].initButtonUL(&lcd, x += btnWidth + btnPad, y, btnWidth, btnHeight, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "-", 1, 1);
	key[15].initButtonUL(&lcd, 255, y, btnWidth, btnHeight, ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE, "X", 1, 1);
	
	refresh = 1;
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