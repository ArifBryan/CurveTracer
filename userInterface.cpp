#include "userInterface.h"
#include "system.h"
#include "Fonts/FreeSans9pt7b.h"
#include "Fonts/FreeSansBold9pt7b.h"
#include "Fonts/FreeSansOblique9pt7b.h"
#include "ctLogoBitmap.h"
#include "outputControl.h"
#include "curveTracer.h"
#include <stdio.h>

UserInterface_TypeDef ui;

char strbuff[200];

uint8_t uiRedraw = 1;
uint8_t uiUpdate = 1;
uint8_t uiNotify;
uint32_t uiTimer;
uint8_t uiUpdateIndex;
volatile uint32_t beepTimer;
uint32_t beepTime;
volatile uint8_t beepCount;
uint32_t touchTimer;
uint8_t uiMenuIndex;
uint8_t editVar;

float vstart, vend, vstep, irange;

Point_TypeDef tsPos;

Adafruit_GFX_Button btn1;
Adafruit_GFX_Button btn2;
Adafruit_GFX_Button btn3;

TextBox_TypeDef text1;
TextBox_TypeDef text2;
TextBox_TypeDef text3;
TextBox_TypeDef text4;
TextBox_TypeDef text5;
TextBox_TypeDef text6;

Plot_TypeDef plot;

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
	
	Point_TypeDef min, max;
	min.x = 0;
	min.y = 0;
	min.z = 70;
	max.x = 320;
	max.y = 240;
	max.z = min.z;
	
	ts.SetCalibration(min, max);
	keypad.Init();
	SetScreenMenu(0);
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
			btn3.press(btn3.contains(tsPos.x, tsPos.y));
			text1.TouchHandler(tsPos, 1);
			text2.TouchHandler(tsPos, 1);
			text3.TouchHandler(tsPos, 1);
			text4.TouchHandler(tsPos, 1);
			text5.TouchHandler(tsPos, 1);
			text6.TouchHandler(tsPos, 1);
		}
	}
	else {
		keypad.TouchHandler(tsPos, 0);
		btn1.press(0);
		btn2.press(0);
		btn3.press(0);
		text1.TouchHandler(tsPos, 0);
		text2.TouchHandler(tsPos, 0);
		text3.TouchHandler(tsPos, 0);
		text4.TouchHandler(tsPos, 0);
		text5.TouchHandler(tsPos, 0);
		text6.TouchHandler(tsPos, 0);
		tsPos.x = 0;
		tsPos.y = 0;
		tsPos.z = 0;
		touchTimer = sys.Ticks();
	}
		
	// Button handler
	ButtonHandler();
	keypad.Handler();
	
	// Display
	if (sys.IsStartup()) {
		uiRedraw = 1;
	}
	if (sys.Ticks() - uiTimer >= 250 || uiRedraw) {
		uiUpdate = 1;
		
		uiTimer = sys.Ticks();
	}
	ScreenMenu();
}


void UserInterface_TypeDef::SetScreenMenu(uint8_t index) {
	uiMenuIndex = index;
	uiUpdateIndex = 0;
	uiRedraw = 1;
	lcd.fillRect(0, 25, 320, 215, ILI9341_WHITE);
}

void UserInterface_TypeDef::ButtonHandler() {	
	if (keypad.IsEnabled()) {
		if (keypad.IsOKPressed()) {
			switch (uiMenuIndex) {
			case 0:
				switch (editVar) {
				case 1:
					outCtl.ch1.SetVoltage(keypad.GetKeyFloat());
					break;
				case 2:
					outCtl.ch2.SetVoltage(keypad.GetKeyFloat());
					break;
				case 3:
					outCtl.ch3.SetVoltage(keypad.GetKeyFloat());
					break;
				case 4:
					outCtl.ch1.SetCurrent(keypad.GetKeyFloat());
					break;
				case 5:
					outCtl.ch2.SetCurrent(keypad.GetKeyFloat());
					break;
				case 6:
					outCtl.ch3.SetCurrent(keypad.GetKeyFloat());
					break;
				}
				break;
			case 1:
				switch (editVar) {
				case 1:
					vstart = keypad.GetKeyFloat();
					break;
				case 2:
					vend = keypad.GetKeyFloat();
					break;
				case 3:
					vstep = keypad.GetKeyFloat();
					break;
				case 4:
					irange = keypad.GetKeyFloat();
					break;
				}
				break;
			}
		}
		if (keypad.IsPressed()) {
			Beep(50);
			if (keypad.IsOKPressed() || keypad.IsCancelPressed()) {
				uiRedraw = 1;
				editVar = 0;
				keypad.Disable();
			}
		}
	}
	if(!keypad.IsEnabled()) {
		switch (uiMenuIndex) {
		case 0:
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
				}
			}
		
		
			if (text1.JustPressed()) {
				Beep(50);
				keypad.Enable("CH1 Vset", "mV");
				editVar = 1;
			}
			if (text2.JustPressed()) {
				Beep(50);
				keypad.Enable("CH2 Vset", "mV");
				editVar = 2;
			}
			if (text3.JustPressed()) {
				Beep(50);
				keypad.Enable("CH3 Vset", "mV");
				editVar = 3;
			}
			if (text4.JustPressed()) {
				Beep(50);
				keypad.Enable("CH1 Iset", "mA");
				editVar = 4;
			}
			if (text5.JustPressed()) {
				Beep(50);
				keypad.Enable("CH2 Iset", "mA");
				editVar = 5;
			}
			if (text6.JustPressed()) {
				Beep(50);
				keypad.Enable("CH3 Iset", "mA");
				editVar = 6;
			}
		
			if (!keypad.IsEnabled()) {
				if (btn1.justPressed() || btn1.justReleased()) {
					btn1.drawButton(btn1.isPressed());
				}
				if (btn2.justPressed() || btn2.justReleased()) {
					btn2.drawButton(btn2.isPressed());
				}
			}
		
			if (btn2.justPressed()) {
				Beep(50);
				SetScreenMenu(1);
			}
			break;
		case 1:
			
			if (text1.JustPressed()) {
				Beep(50);
				keypad.Enable("V Start", "mV");
				editVar = 1;
			}
			if (text2.JustPressed()) {
				Beep(50);
				keypad.Enable("V End", "mV");
				editVar = 2;
			}
			if (text3.JustPressed()) {
				Beep(50);
				keypad.Enable("V Step", "mV");
				editVar = 3;
			}
			if (text4.JustPressed()) {
				Beep(50);
				keypad.Enable("I Range", "mA");
				editVar = 4;
			}
			
			if (!keypad.IsEnabled()) {
				if (btn1.justPressed() || btn1.justReleased()) {
					btn1.drawButton(btn1.isPressed());
				}
				if (btn2.justPressed() || btn2.justReleased()) {
					btn2.drawButton(btn2.isPressed());
				}
			}
			
			
			if (btn1.justPressed()) {
				Beep(50);
				
				curveTracer.SetupChannel(&outCtl.ch1, &outCtl.ch2, &outCtl.ch3);
				curveTracer.SetupParams(vstart, vend, vstep, irange, 100);
				curveTracer.Start();
				plot.ResetLinePlot();
				SetScreenMenu(2);
			}
			if (btn2.justPressed()) {
				Beep(50);
				SetScreenMenu(0);
			}
			break;
		case 2:
			if (curveTracer.IsSamplingDone() && curveTracer.IsNewSample()) {
				btn1.setLabel("START");
				btn1.setColor(ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE);
				btn1.drawButton(btn1.isPressed());
				ui.Beep(50, 2);
				uiNotify = 1;
			}
			if (curveTracer.IsNewSample(1)) {
				uiUpdate = 1;
			}
			if (btn1.justPressed()) {
				Beep(50);
		
				if (!curveTracer.IsSampling()) {
					plot.ResetLinePlot();
					curveTracer.Start();
					btn1.setLabel("STOP");
					btn1.setColor(ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE);
				}
				else {
					curveTracer.Stop();
					btn1.setLabel("START");
					btn1.setColor(ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE);
				}
				uiNotify = 1;
			}
			if (btn3.justPressed()) {
				Beep(50);
				curveTracer.Stop();
				plot.Clear();
				uiRedraw = 1;
				uiNotify = 1;
			}
		
			if (!keypad.IsEnabled()) {
				if (btn1.justPressed() || btn1.justReleased()) {
					btn1.drawButton(btn1.isPressed());
				}
				if (btn2.justPressed() || btn2.justReleased()) {
					btn2.drawButton(btn2.isPressed());
				}
				if (btn3.justPressed() || btn3.justReleased()) {
					btn3.drawButton(btn3.isPressed());
				}
			}
		
			if (btn2.justPressed()) {
				Beep(50);
				curveTracer.Stop();
				SetScreenMenu(1);
			}
			break;
		}
	}
}

void UserInterface_TypeDef::ScreenMenu() {
	if (uiRedraw || uiUpdate || uiNotify) {
		GFXcanvas16 canvas(320, 25);
		canvas.setFont(&FreeSans9pt7b);
		switch (uiMenuIndex) {
		case 0:
			canvas.fillScreen(ILI9341_DARKCYAN);
			sprintf(strbuff, "Source & Measure");
			break;
		case 1:
			canvas.fillScreen(ILI9341_DARKCYAN);
			sprintf(strbuff, "Trace Setup");
			break;
		case 2:
			if (curveTracer.IsSampling()) {
				canvas.fillScreen(ILI9341_DARKGREEN);
				sprintf(strbuff, "Trace - Running");
			}
			else {
				canvas.fillScreen(ILI9341_DARKCYAN);
				sprintf(strbuff, "Trace");
			}
			break;
		}
		canvas.setCursor(5, 17);
		canvas.print(strbuff);
		canvas.setCursor(267, 17);
		canvas.setTextColor(sys.OverTemperature() ? ILI9341_RED : ILI9341_WHITE);
		sprintf(strbuff, "%d.%dC\n", (uint8_t)sys.ReadDriverTemp(), (uint16_t)(sys.ReadDriverTemp() * 10) % 10);
		canvas.print(strbuff);
		lcd.drawRGBBitmap(0, 0, canvas.getBuffer(), 320, 25);
	}
	if (!keypad.IsEnabled()) {
		switch (uiMenuIndex) {
		case 0:
			if (uiRedraw) {
				if (outCtl.ch1.GetState() || outCtl.ch2.GetState() || outCtl.ch3.GetState()) {
					btn1.initButton(&lcd, 275, 55, 75, 45, ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE, "ON", 1, 1);
				}
				else {
					btn1.initButton(&lcd, 275, 55, 75, 45, ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE, "OFF", 1, 1);					
				}
				btn2.initButton(&lcd, 275, 210, 75, 45, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "TRACE", 1, 1);
				text1.Init(45, 30, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mV");
				text2.Init(45, 55, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mV");
				text3.Init(45, 80, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mV");
				text4.Init(140, 30, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mA");
				text5.Init(140, 55, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mA");
				text6.Init(140, 80, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mA");
				
				btn1.drawButton(btn1.isPressed());
				btn2.drawButton(btn2.isPressed());
				
				GFXcanvas16 canvas(40, 100);
				
				canvas.fillScreen(ILI9341_WHITE);
				canvas.setFont(&FreeSans9pt7b);
				canvas.setTextColor(ILI9341_DARKGREY);
				canvas.setCursor(0, 18);
				canvas.print("CH1");
				canvas.setCursor(0, 43);
				canvas.print("CH2");
				canvas.setCursor(0, 68);
				canvas.print("CH3");
				lcd.drawRGBBitmap(3, 27, canvas.getBuffer(), 40, 100);
			}
			if (uiRedraw || uiUpdate) {
				
				if (outCtl.ch1.GetState()) {
					text1.Draw((int16_t)outCtl.ch1.GetVoltage());
					text4.Draw((int16_t)outCtl.ch1.GetCurrent());
				}
				else {
					text1.Draw((int16_t)outCtl.ch1.GetSetVoltage());
					text4.Draw((int16_t)outCtl.ch1.GetSetCurrent());
				}
				if (outCtl.ch2.GetState()) {
					text2.Draw((int16_t)outCtl.ch2.GetVoltage());
					text5.Draw((int16_t)outCtl.ch2.GetCurrent());
				}
				else {
					text2.Draw((int16_t)outCtl.ch2.GetSetVoltage());
					text5.Draw((int16_t)outCtl.ch2.GetSetCurrent());
				}
				if (outCtl.ch3.GetState()) {
					text3.Draw((int16_t)outCtl.ch3.GetVoltage());
					text6.Draw((int16_t)outCtl.ch3.GetCurrent());
				}
				else {
					text3.Draw((int16_t)outCtl.ch3.GetSetVoltage());
					text6.Draw((int16_t)outCtl.ch3.GetSetCurrent());
				}
			}
			break;
		case 1:
			if (uiRedraw) {
				btn1.initButton(&lcd, 275, 55, 75, 45, ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE, "START", 1, 1);
				btn2.initButton(&lcd, 275, 210, 75, 45, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "SMU", 1, 1);
				text1.Init(60, 30, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mV");
				text2.Init(60, 55, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mV");
				text3.Init(60, 80, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mV");
				text4.Init(60, 105, 60, 19, ILI9341_DARKGREY, ILI9341_DARKCYAN, "mA");
					
				btn1.drawButton(btn1.isPressed());
				btn2.drawButton(btn2.isPressed());
				
				GFXcanvas16 canvas(60, 100);
				
				canvas.fillScreen(ILI9341_WHITE);
				canvas.setFont(&FreeSans9pt7b);
				canvas.setTextColor(ILI9341_DARKGREY);
				canvas.setCursor(0, 18);
				canvas.print("Start");
				canvas.setCursor(0, 43);
				canvas.print("End");
				canvas.setCursor(0, 68);
				canvas.print("Step");
				canvas.setCursor(0, 93);
				canvas.print("Range");
				lcd.drawRGBBitmap(3, 27, canvas.getBuffer(), 60, 100);
				
				text1.Draw((int16_t)vstart);
				text2.Draw((int16_t)vend);
				text3.Draw(vstep);
				text4.Draw((int16_t)irange);
			}
			if (uiRedraw || uiUpdate) {
				
			}
			break;
		case 2:
			if (uiRedraw) {
				if (curveTracer.IsSampling()) {
					btn1.initButton(&lcd, 275, 55, 75, 45, ILI9341_MAROON, ILI9341_MAROON, ILI9341_WHITE, "STOP", 1, 1);
				}
				else {
					btn1.initButton(&lcd, 275, 55, 75, 45, ILI9341_DARKGREEN, ILI9341_DARKGREEN, ILI9341_WHITE, "START", 1, 1);
				}
				btn2.initButton(&lcd, 275, 210, 75, 45, ILI9341_DARKCYAN, ILI9341_DARKCYAN, ILI9341_WHITE, "BACK", 1, 1);
				btn3.initButton(&lcd, 275, 105, 75, 45, ILI9341_ORANGE, ILI9341_ORANGE, ILI9341_WHITE, "CLEAR", 1, 1);
					
				btn1.drawButton(btn1.isPressed());
				btn2.drawButton(btn2.isPressed());
				btn3.drawButton(btn3.isPressed());
				
				plot.Init(20, 36, 195, 188, "mV", "mA", vstart, vend, 0, irange, 10, 10);
				plot.SetPlotColor(ILI9341_ORANGE);
			}
			if (uiRedraw || uiUpdate) {
				if (curveTracer.GetSampleCount() > 0) {
					plot.DrawLine(
						curveTracer.data[curveTracer.GetSampleCount() - 1].v, 
						curveTracer.data[curveTracer.GetSampleCount() - 1].i);
				}
			}
			break;
		}
	}
	uiRedraw = 0;
	uiUpdate = 0;
	uiNotify = 0;
}

// Plot
void Plot_TypeDef::DrawPoint(float xVal, float yVal) {
	uint16_t gx = xPos + 1, gy = yPos + h - 1;
	
	if ((xVal >= xMin && xVal <= xMax) && (yVal >= yMin && yVal <= yMax)) {
		xVal -= xMin;
		yVal -= yMin;
		xVal = xVal / (xMax - xMin);
		yVal = yVal / (yMax - yMin);
				
		uint16_t x = round(gx + xVal * w);
		uint16_t y = round(gy - yVal * h);
		
		if ((x < xPos || x > xPos + w) || (y < yPos || y > yPos + h)) {return;}
		
		lcd.drawPixel(x, y, plotColor);
	}
}
void Plot_TypeDef::DrawLine(float x1Val, float y1Val) {
	uint16_t gx = xPos + 1, gy = yPos + h - 1;
	
	if ((x1Val >= xMin && x1Val <= xMax) && (y1Val >= yMin && y1Val <= yMax)) {
		x1Val -= xMin;
		y1Val -= yMin;
		x1Val = x1Val / (xMax - xMin);
		y1Val = y1Val / (yMax - yMin);
		
		uint16_t x = round(gx + x1Val * w);
		uint16_t y = round(gy - y1Val * h);
		
		if ((x < xPos || x > xPos + w) || (y < yPos || y > yPos + h)) {return;}
		
		if (lPoint[0] == 0 && lPoint[1] == 0) {
			lPoint[0] = x;
			lPoint[1] = y;
		}
		lcd.drawLine(lPoint[0], lPoint[1], x, y, plotColor);
		lPoint[0] = x;
		lPoint[1] = y;
	}
}
void Plot_TypeDef::DrawLine(float x1Val, float y1Val, float x2Val, float y2Val) {
	uint16_t gx = xPos + 1, gy = yPos + h - 1;
	
	if ((x1Val >= xMin && x1Val <= xMax) && (y1Val >= yMin && y1Val <= yMax) &&
		(x2Val >= xMin && x2Val <= xMax) && (y2Val >= yMin && y2Val <= yMax)) {
		x1Val -= xMin;
		y1Val -= yMin;
		x1Val = x1Val / (xMax - xMin);
		y1Val = y1Val / (yMax - yMin);
		x2Val -= xMin;
		y2Val -= yMin;
		x2Val = x2Val / (xMax - xMin);
		y2Val = y2Val / (yMax - yMin);
			
		uint16_t x1 = round(gx + x1Val * w);
		uint16_t y1 = round(gy - y1Val * h);
			
		uint16_t x2 = round(gx + x2Val * w);
		uint16_t y2 = round(gy - y2Val * h);
			
		
		if ((x1 < xPos || x1 > xPos + w) || (y1 < yPos || y1 > yPos + h)) {return;}
		if ((x2 < xPos || x2 > xPos + w) || (y2 < yPos || y2 > yPos + h)) {return;}
		
		lcd.drawLine(x1, y1, x2, y2, plotColor);
	}
}
void Plot_TypeDef::ResetLinePlot() {
	lPoint[0] = 0;
	lPoint[1] = 0;
}
void Plot_TypeDef::Clear() {
	lcd.fillRect(xPos + 1, yPos, w, h, ILI9341_WHITE);
}
void Plot_TypeDef::Init(uint16_t xPos, uint16_t yPos, uint16_t w, uint16_t h, const char* xLabel, const char* yLabel, float xMin, float xMax, float yMin, float yMax, uint8_t xDiv, uint8_t yDiv) {
	uint16_t tw, th;
	int16_t x1, y1;
	char txt[9];
	
	w += 1;
	h += 1;
	
	this->xPos = xPos;
	this->yPos = yPos;
	this->w = w;
	this->h = h;
	this->xMin = xMin;
	this->xMax = xMax;
	this->yMin = yMin;
	this->yMax = yMax;
	
	lcd.setFont(0);
	lcd.setTextColor(ILI9341_DARKGREY);
	lcd.drawFastHLine(xPos, yPos + h, w, ILI9341_DARKGREY);
	lcd.drawFastVLine(xPos, yPos, h, ILI9341_DARKGREY);
	lcd.setCursor(xPos - 5, yPos - 9);
	lcd.print(yLabel);
	lcd.setCursor(xPos + w + 3, yPos + h - 5);
	lcd.print(xLabel);
	lcd.drawFastHLine(xPos - 1, yPos, 1, ILI9341_DARKGREY);
	lcd.drawFastHLine(xPos - 1, yPos + h, 1, ILI9341_DARKGREY);
	sprintf(txt, "%d", (int16_t)yMax);
	lcd.getTextBounds(txt, xPos, yPos, &x1, &y1, &tw, &th);
	lcd.setCursor(xPos - tw, yPos);
	lcd.print(txt);
	sprintf(txt, "%d", (int16_t)yMin);
	lcd.getTextBounds(txt, xPos, yPos, &x1, &y1, &tw, &th);
	lcd.setCursor(xPos - tw, yPos + h - 6);
	lcd.print(txt);
	sprintf(txt, "%d", (int16_t)xMin);
	lcd.getTextBounds(txt, xPos, yPos, &x1, &y1, &tw, &th);
	lcd.setCursor(xPos, yPos + h + 2);
	lcd.print(txt);
	sprintf(txt, "%d", (int16_t)xMax);
	lcd.getTextBounds(txt, xPos, yPos, &x1, &y1, &tw, &th);
	lcd.setCursor(xPos + w - tw, yPos + h + 2);
	lcd.print(txt);
	lcd.drawFastVLine(xPos, yPos + h + 1, 1, ILI9341_DARKGREY);
	lcd.drawFastVLine(xPos + w - 1, yPos + h + 1, 1, ILI9341_DARKGREY);
	w -= 1;
	h -= 1;
	for (uint8_t i = 1; i <= xDiv && (xMax - xMin) > 0; i++) {
		float xVal = round((xMax - xMin) / xDiv) * i;
		xVal = xVal / (xMax - xMin);
		uint16_t x = round(xPos + xVal * w);
		lcd.drawFastVLine(x, yPos, h + 1, ILI9341_LIGHTGREY);
	}
	for (uint8_t i = 1; i <= yDiv && (yMax - yMin) > 0; i++) {
		float yVal = round((yMax - yMin) / yDiv) * i;
		yVal = yVal / (yMax - yMin);
		uint16_t y = round(yPos + h - yVal * h);
		lcd.drawFastHLine(xPos + 1, y, w, ILI9341_LIGHTGREY);
	}
	lcd.setFont(&FreeSans9pt7b);
}

// TextBox
void TextBox_TypeDef::TouchHandler(Point_TypeDef pos, bool touch) {
	int16_t px = pos.x;
	int16_t py = pos.y;
	
	stateLast = stateNow;
	if (((px >= x) && (px < (int16_t)(x + w)) && (py >= y) && (py < (int16_t)(y + h))) && touch) {
		stateNow = 1;
	}
	else {stateNow = 0; }
}
void TextBox_TypeDef::Draw(float num) {
	char str[21];
	sprintf(str, "%d.%1d", (int16_t)num, (uint16_t)(abs(num) * 10) % 10);
	Draw(str);
}
void TextBox_TypeDef::Draw(int num) {
	char str[21];
	sprintf(str, "%d", num);
	Draw(str);
}
void TextBox_TypeDef::Draw(const char* str) {
	int16_t x1, y1;
	uint16_t tw, th;
	GFXcanvas16 box(w, h);
	
	box.fillScreen(ILI9341_WHITE);
	box.setFont(&FreeSans9pt7b);
	box.setTextColor(textColor);
	box.drawRoundRect(0, 0, w, h, 2, color);
	box.getTextBounds(str, 0, 0, &x1, &y1, &tw, &th);
	box.setCursor(w - tw - 6, th + 2);
	box.print(str);
	lcd.drawRGBBitmap(x, y, box.getBuffer(), w, h);
}
void TextBox_TypeDef::Init(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t textColor, const char* unit) {
	int16_t x1, y1;
	uint16_t tw, th;
	lcd.getTextBounds(unit, 0, 0, &x1, &y1, &tw, &th);
	tw += 1;
	GFXcanvas16 unitText(tw, th);
	
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	this->color = color;
	this->textColor = textColor;
	unitText.fillScreen(ILI9341_WHITE);
	unitText.setFont(&FreeSans9pt7b);
	unitText.setTextColor(color);
	unitText.setCursor(0, th);
	unitText.print(unit);
	lcd.drawRGBBitmap(x + w + 1, y + 2, unitText.getBuffer(), tw, th);
}

// Touch Overlay
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

// Keypad
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
		if (refresh) {
			int16_t x1, y1;
			uint16_t tw, th;
			lcd.getTextBounds(unit, 0, 0, &x1, &y1, &tw, &th);
			tw += 1;
			GFXcanvas16 unitText(tw, 18);
			
			unitText.fillScreen(ILI9341_WHITE);
			unitText.setTextColor(ILI9341_DARKGREY);
			unitText.setFont(&FreeSans9pt7b);
			unitText.setCursor(0, 17);
			unitText.print(unit);
			
			lcd.drawRGBBitmap(221, 31, unitText.getBuffer(), tw, 18);
		}
		if (IsPressed() || refresh) {
			refresh = 1;
			if (IsPressed()) {
				if (keyLUT[pressedKey - 1] == 'O' || keyLUT[pressedKey - 1] == 'C') {
					//Disable();
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
				if (keyBufferPtr == 0) {
					textBox.setCursor(textBox.getCursorX(), 18);
					textBox.setTextColor(ILI9341_LIGHTGREY);
					textBox.print(label);
				}
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
	return atof(keyBuffer);
}
int Keypad_TypeDef::GetKeyInteger() {
	return atoi(keyBuffer);
}

uint8_t Keypad_TypeDef::IsPressed() {
	return pressedKey > 0;
}

void Keypad_TypeDef::Enable(const char* label, const char* unit) {
	if (!enable) {
		enable = 1;
		refresh = 1;
		decPoint = 0;
		keyBufferPtr = 0;
		strcpy(this->label, label);
		strcpy(this->unit, unit);
		memset(keyBuffer, 0, 19);
		lcd.fillRect(0, 25, 320, 215, ILI9341_WHITE);
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