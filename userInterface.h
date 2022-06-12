#pragma once

#include <stm32f1xx.h>
#include "system.h"

struct Plot_TypeDef {
	void Init(uint16_t xPos, uint16_t yPos, uint16_t w, uint16_t h, const char* xLabel, const char* yLabel, float xMin, float xMax, float yMin, float yMax, uint8_t xDiv, uint8_t yDiv);
	void SetPlotColor(uint16_t color) {
		plotColor = color;
	}
	void DrawPoint(float xVal, float yVal);
	void DrawLine(float x1Val, float y1Val, float x2Val, float y2Val);
	void DrawLine(float xVal, float yVal);
	void ResetLinePlot(void);
	void Clear(void);
private:
	uint16_t xPos, yPos;
	uint16_t w, h;
	uint16_t plotColor;
	uint16_t lPoint[2];
	float xMin, xMax;
	float yMin, yMax;
};

struct TextBox_TypeDef {
	void Init(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color, uint16_t textColor, const char* unit);
	void SetTextColor(uint16_t textColor) {
		this->textColor = textColor;
	}
	void Draw(const char* str);
	void Draw(int num);
	void Draw(float num);
	void TouchHandler(Point_TypeDef pos, bool touch);
	bool IsPressed(void) {return stateNow;}
	bool JustPressed(void) {return stateNow && !stateLast;}
private:
	uint16_t x, y, w, h;
	uint16_t color, textColor;
	bool stateNow;
	bool stateLast;
};

struct TouchOverlay_TypeDef {
	void TouchHandler(Point_TypeDef pos, bool touch);
	void Init(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
	bool IsPressed(void) {return stateNow;}
	bool JustPressed(void) {return stateNow && !stateLast;}
	uint16_t GetPositionX(void) {return x;}
	uint16_t GetPositionY(void) {return y;}
	uint16_t GetWidth(void) {return w;}
	uint16_t GetHeight(void) {return h;}
private:
	bool stateNow;
	bool stateLast;
	int16_t x, y;
	uint16_t w, h;
};

struct Keypad_TypeDef {
	void Init(void);
	void TouchHandler(Point_TypeDef pos, uint8_t touch);
	void Handler(void);
	uint8_t IsPressed(void);
	char GetKey(void) {
		if (pressedKey > 0) {
			return keyLUT[pressedKey - 1];
		}
		return 0;
	}
	void ClearKey(void) {pressedKey = 0;}
	char* GetKeyString(void) {return keyBuffer;}
	float GetKeyFloat(void);
	int GetKeyInteger(void);
	uint8_t IsOKPressed(void) {return GetKey() == 'O';}
	uint8_t IsCancelPressed(void) {return GetKey() == 'C';}
	uint8_t IsEnabled(void);
	void Enable(const char* label, const char* unit);
	void Disable(void);
private:
	Adafruit_GFX_Button key[16];
	uint8_t enable;
	uint8_t refresh;
	uint8_t decPoint;
	uint8_t pressedKey;
	char keyBuffer[19];
	uint8_t keyBufferPtr;
	char label[21];
	char unit[11];
	const char keyLUT[16] = { 
		'1', '2', '3', 'D',
		'4', '5', '6', 'A',
		'7', '8', '9', 'O',
		'.', '0', '-', 'C'
	};
};

struct UserInterface_TypeDef {
	void Init(void);
	void Handler(void);
	void SetBrightness(uint8_t bright);
	uint32_t GetBrightness();
	void Beep(uint32_t t, uint8_t cnt = 1);
	uint8_t IsTouched(void);
	void GetTouchPosition(uint16_t *x, uint16_t *y);
	void Ticks10ms_IRQ_Handler();
	void ScreenMenu(void);
	void SetScreenMenu(uint8_t index);
	void ButtonHandler(void);
	void ForceRedraw(void) {
		uiRedraw = 1;
	}
	Keypad_TypeDef keypad;
private:
	void SplashScreen(void);
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
	uint8_t sysMenu;
};

extern UserInterface_TypeDef ui;