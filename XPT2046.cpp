#include "XPT2046.h"
#include <stm32f1xx_ll_exti.h>

void XPT2046_TypeDef::Init() {
	pointPos.x = 0;
	pointPos.y = 0;
	pointPos.z = 0;
}

void XPT2046_TypeDef::StartConversion() {
	StartWrite();
	uint16_t lzPos = pointPos.z;
	Write(XPT2046_READ_Z1POS);
	pointPos.z = Read();
	if (pointPos.z > 50) {
		Write(XPT2046_READ_XPOS);
		pointPos.x = Read();
		Write(XPT2046_READ_YPOS);
		pointPos.y = Read();
	}
	else {
		pointPos.z = lzPos;
	}
	EndWrite();
}

void XPT2046_TypeDef::GetPosition(Point_TypeDef *point) {
	switch (rotation) {
	case 1:
		point->x = 4095 - pointPos.x;
		point->y = 4095 - pointPos.y;
		break;
	}
}

void XPT2046_TypeDef::SetRotation(uint8_t r) {
	rotation = r;
	switch (rotation) {
	case 1:
		pointPos.x = 4095 - pointPos.x;
		pointPos.y = 4095 - pointPos.y;
		break;
	}
}

uint16_t XPT2046_TypeDef::Read() {
	// !IMPORTANT! Flush buffer before read operation !IMPORTANT! //
	LL_SPI_ReceiveData16(spi);
	
	LL_SPI_SetDataWidth(spi, LL_SPI_DATAWIDTH_16BIT);
	LL_SPI_TransmitData16(spi, 0);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	return LL_SPI_ReceiveData16(spi) >> 3;
}

void XPT2046_TypeDef::Write(uint8_t data) {
	LL_SPI_SetDataWidth(spi, LL_SPI_DATAWIDTH_8BIT);
	LL_SPI_TransmitData8(spi, data);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
}

uint8_t XPT2046_TypeDef::IsSPIBusy() {
	return LL_SPI_IsActiveFlag_BSY(spi);
}

void XPT2046_TypeDef::StartWrite() {
	while (IsSPIBusy()) ;
	if (extiLine) {
		LL_EXTI_DisableIT_0_31(extiLine);
	}
	LL_GPIO_ResetOutputPin(csGPIO, csPIN);
	spiLastSpeed = LL_SPI_GetBaudRatePrescaler(spi);
	spiLastDataWidth = LL_SPI_GetDataWidth(spi);
	LL_SPI_SetBaudRatePrescaler(spi, LL_SPI_BAUDRATEPRESCALER_DIV64);
}

void XPT2046_TypeDef::EndWrite() {
	LL_GPIO_SetOutputPin(csGPIO, csPIN);
	LL_SPI_SetBaudRatePrescaler(spi, spiLastSpeed);
	LL_SPI_SetDataWidth(spi, spiLastDataWidth);
	if (extiLine) {
		LL_EXTI_EnableIT_0_31(extiLine); 
	}
}