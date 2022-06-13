#include "XPT2046.h"
#include <stm32f1xx_ll_exti.h>

void XPT2046_TypeDef::Init() {
	pointPos.x = 0;
	pointPos.y = 0;
	pointPos.z = 0;
	spiTransCount = 0;
	pCalMin.x = 0;
	pCalMin.y = 0;
	pCalMin.z = 0;
	pCalMax.x = 4096;
	pCalMax.y = 4096;
	pCalMax.z = 0;
}

void XPT2046_TypeDef::SPI_IRQ_Handler() {
	if(LL_SPI_IsActiveFlag_TXE(spi) && !LL_SPI_IsActiveFlag_BSY(spi) && spiTransCount){
		spiTransCount--;
		switch (spiTransCount) {
		case 6:
			Write(0);
			break;
		case 5:
			pointPos.z = Read8() << 5;
			Write(XPT2046_READ_XPOS);
			break;
		case 4:
			pointPos.z |= Read8() >> 3;
			Write(0);
			break;
		case 3:
			pointPos.x = Read8() << 5;
			Write(XPT2046_READ_YPOS);
			break;
		case 2:
			pointPos.x |= Read8() >> 3;
			Write(0);
			break;
		case 1:
			pointPos.y = Read8() << 5;
			Write(0);
			break;
		case 0:
			pointPos.y |= Read8() >> 3;
			EndWrite();
			LL_SPI_DisableIT_TXE(spi);
			newData = 1;
			break;
		}
	}
}

void XPT2046_TypeDef::StartConversion() {
	StartWrite();
	if (spiInterrupt) {
		spiTransCount = 7;
		newData = 0;
		pointPos.x = 0;
		pointPos.y = 0;
		pointPos.z = 0;
		Write(XPT2046_READ_Z1POS);
		LL_SPI_EnableIT_TXE(spi);
	}
	else {
		Write(XPT2046_READ_Z1POS);
		pointPos.z = Read16() >> 3;
		Write(XPT2046_READ_XPOS);
		pointPos.x = Read16() >> 3;
		Write(XPT2046_READ_YPOS);
		pointPos.y = Read16() >> 3;
		EndWrite();
	}
}

void XPT2046_TypeDef::GetPosition(Point_TypeDef *point) {
	switch (rotation) {
	case 1:
		point->y = 4095 - pointPos.x;
		point->x = 4095 - pointPos.y;
		break;
	case 2:
		point->x = 4095 - pointPos.x;
		point->y = 4095 - pointPos.y;
		break;
	}
	point->z = pointPos.z;
	
	point->x = (point->x - pCalMin.x) / (4096 / (pCalMax.x - pCalMin.x));
	point->y = (point->y - pCalMin.y) / (4096 / (pCalMax.y - pCalMin.y));
	
	pointPos.x = 0;
	pointPos.y = 0;
	pointPos.z = 0;
}

uint8_t XPT2046_TypeDef::IsTouched() {
	return pointPos.z > zThreshold && newData;
}

void XPT2046_TypeDef::SetCalibration(Point_TypeDef pMin, Point_TypeDef pMax) {
	pCalMin = pMin;
	pCalMax = pMax;
	zThreshold = (pMin.z <= pMax.z ? pMin.z : pMax.z) * 0.75;
}

void XPT2046_TypeDef::SetRotation(uint8_t r) {
	rotation = r;
}

uint16_t XPT2046_TypeDef::Read16() {
	if (!spiInterrupt) {
		// !IMPORTANT! Flush buffer before read operation !IMPORTANT! //
		LL_SPI_ReceiveData16(spi);
	
		LL_SPI_SetDataWidth(spi, LL_SPI_DATAWIDTH_16BIT);
		LL_SPI_TransmitData16(spi, 0);
		while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	}
	return LL_SPI_ReceiveData16(spi);
}

uint8_t XPT2046_TypeDef::Read8() {
	if (!spiInterrupt) {
		// !IMPORTANT! Flush buffer before read operation !IMPORTANT! //
		LL_SPI_ReceiveData8(spi);
	
		LL_SPI_SetDataWidth(spi, LL_SPI_DATAWIDTH_8BIT);
		LL_SPI_TransmitData8(spi, 0);
		while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	}
	return LL_SPI_ReceiveData8(spi);
}

void XPT2046_TypeDef::Write(uint8_t data) {
	LL_SPI_SetDataWidth(spi, LL_SPI_DATAWIDTH_8BIT);
	// !IMPORTANT! Flush buffer before read operation !IMPORTANT! //
	LL_SPI_ReceiveData8(spi);
	LL_SPI_TransmitData8(spi, data);
	if (!spiInterrupt) {
		while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	}
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