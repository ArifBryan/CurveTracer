#pragma once

#include <stm32f1xx.h>
#include "system.h"

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
	uint8_t IsEnabled(void);
	void Enable(void);
	void Disable(void);
private:
	Adafruit_GFX_Button key[16];
	uint8_t enable;
	uint8_t refresh;
	uint8_t decPoint;
	uint8_t pressedKey;
	char keyBuffer[19];
	uint8_t keyBufferPtr;
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
	void Beep(uint32_t t, uint8_t cnt = 1);
	uint8_t IsTouched(void);
	void GetTouchPosition(uint16_t *x, uint16_t *y);
	void Ticks10ms_IRQ_Handler();
	void ScreenMenu(void);
	void SetScreenMenu(uint8_t index);
	void ButtonHandler(void);
	Keypad_TypeDef keypad;
private:
	void SplashScreen(void);
};

extern UserInterface_TypeDef ui;