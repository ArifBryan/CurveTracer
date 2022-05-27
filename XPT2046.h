#pragma once

#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_spi.h>

#define XPT2046_INPUT_YPOS	0b001
#define XPT2046_INPUT_Z1POS	0b011
#define XPT2046_INPUT_Z2POS	0b100
#define XPT2046_INPUT_XPOS	0b101
#define XPT2046_MODE_12BIT	0b0
#define XPT2046_MODE_8BIT	0b1
#define XPT2046_MODE_DIFF	0b0
#define XPT2046_MODE_SINGLE	0b1
#define XPT2046_PD_NORMAL	0b00
#define XPT2046_PD_NOIRQ	0b01
#define XPT2046_PD_DISABLE	0b11

#define XPT2046_READ_XPOS	(1 << 7) | (XPT2046_INPUT_XPOS << 4) | (XPT2046_MODE_12BIT << 3) | (XPT2046_MODE_DIFF << 2) | (XPT2046_PD_NORMAL)
#define XPT2046_READ_YPOS	(1 << 7) | (XPT2046_INPUT_YPOS << 4) | (XPT2046_MODE_12BIT << 3) | (XPT2046_MODE_DIFF << 2) | (XPT2046_PD_NORMAL)
#define XPT2046_READ_Z1POS	(1 << 7) | (XPT2046_INPUT_Z1POS << 4) | (XPT2046_MODE_12BIT << 3) | (XPT2046_MODE_DIFF << 2) | (XPT2046_PD_NORMAL)
#define XPT2046_READ_Z2POS	(1 << 7) | (XPT2046_INPUT_Z2POS << 4) | (XPT2046_MODE_12BIT << 3) | (XPT2046_MODE_DIFF << 2) | (XPT2046_PD_NORMAL)

struct Point_TypeDef {
	uint16_t x;
	uint16_t y;
	uint16_t z;
};

struct XPT2046_TypeDef {
	XPT2046_TypeDef(SPI_TypeDef *spi, GPIO_TypeDef *csGPIO, uint32_t csPIN, uint32_t EXTI_Line = 0) {
		this->spi = spi;
		this->csGPIO = csGPIO;
		this->csPIN = csPIN;
		this->extiLine = EXTI_Line;
	}
	void Init(void);
	void StartConversion(void);
	void SetCalibration(Point_TypeDef p1, Point_TypeDef p2, Point_TypeDef p3, Point_TypeDef p4);
	void GetPosition(Point_TypeDef *point);
	void SetRotation(uint8_t r);
	void SPI_IRQ_Handler();
private:
	uint8_t IsSPIBusy(void);
	void Write(uint8_t data);
	uint16_t Read(void);
	void StartWrite(void);
	void EndWrite(void);	
	
	SPI_TypeDef *spi;
	GPIO_TypeDef *csGPIO;
	uint32_t csPIN;
	uint32_t extiLine;
	uint32_t spiLastSpeed;
	uint32_t spiLastDataWidth;
	uint8_t rotation;
	
	Point_TypeDef pointPos;
	Point_TypeDef pointCal;
};