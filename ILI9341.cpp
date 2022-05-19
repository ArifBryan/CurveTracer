#include "ILI9341.h"
#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_spi.h>
#include <stm32f1xx_ll_utils.h>

uint16_t _width, _height;
uint8_t rotation;

#define MADCTL_MY 0x80  ///< Bottom to top
#define MADCTL_MX 0x40  ///< Right to left
#define MADCTL_MV 0x20  ///< Reverse Mode
#define MADCTL_ML 0x10  ///< LCD refresh Bottom to top
#define MADCTL_RGB 0x00 ///< Red-Green-Blue pixel order
#define MADCTL_BGR 0x08 ///< Blue-Green-Red pixel order
#define MADCTL_MH 0x04  ///< LCD refresh right to left

const uint8_t initCmd[] = { 
	0xEF, 3, 0x03, 0x80, 0x02,
	0xCF, 3, 0x00, 0xC1, 0x30,
	0xED, 4, 0x64, 0x03, 0x12, 0x81,
	0xE8, 3, 0x85, 0x00, 0x78,
	0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
	0xF7, 1, 0x20,
	0xEA, 2, 0x00, 0x00,
	ILI9341_PWCTR1, 1, 0x23, // Power control VRH[5:0]
	ILI9341_PWCTR2, 1, 0x10, // Power control SAP[2:0];BT[3:0]
	ILI9341_VMCTR1, 2, 0x3e, 0x28, // VCM control
	ILI9341_VMCTR2, 1, 0x86, // VCM control2
	ILI9341_MADCTL, 1, 0x48, // Memory Access Control
	ILI9341_VSCRSADD, 1, 0x00, // Vertical scroll zero
	ILI9341_PIXFMT, 1, 0x55,
	ILI9341_FRMCTR1, 2, 0x00, 0x18,
	ILI9341_DFUNCTR, 3, 0x08, 0x82, 0x27, // Display Function Control
	0xF2, 1, 0x00, // 3Gamma Function Disable
	ILI9341_GAMMASET, 1, 0x01, // Gamma curve selected
	ILI9341_GMCTRP1, 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
	  0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
	ILI9341_GMCTRN1, 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
	  0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
	ILI9341_SLPOUT, 0x80, // Exit Sleep
	ILI9341_DISPON, 0x80, // Display on
	0x00                                   // End of list  
};

void ILI9341_TypeDef::Init() {
	sendCommand(ILI9341_SWRESET);
	LL_mDelay(150);
	
	const uint8_t *cmdPtr = initCmd;
	uint8_t cmd;
	while ((cmd = *cmdPtr++) > 0) {
		uint8_t x = *cmdPtr++;
		uint8_t cmdLen = x & 0x7F;
		sendCommand(cmd, (uint8_t*)cmdPtr, cmdLen);
		cmdPtr += cmdLen;
		if (x & 0x80) {LL_mDelay(150);}
	}
}

void ILI9341_TypeDef::sendCommand(uint8_t cmd, uint8_t *data, uint8_t dataLen) {
	LL_GPIO_ResetOutputPin(csGPIO, csPIN);
	
	LL_GPIO_ResetOutputPin(dcGPIO, dcPIN);
	LL_SPI_TransmitData8(spi, cmd);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	
	LL_GPIO_SetOutputPin(dcGPIO, dcPIN);
	for (uint8_t i = 0; i < dataLen; i++) {
		LL_SPI_TransmitData8(spi, data[i]);
		while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	}
	LL_GPIO_SetOutputPin(csGPIO, csPIN);
	
	_width = ILI9341_TFTWIDTH;
	_height = ILI9341_TFTHEIGHT;
}

void ILI9341_TypeDef::SetRotation(uint8_t m) {
	rotation = m % 4; // can't be higher than 3
	switch (rotation) {
	case 0:
		m = (MADCTL_MX | MADCTL_BGR);
		_width = ILI9341_TFTWIDTH;
		_height = ILI9341_TFTHEIGHT;
		break;
	case 1:
		m = (MADCTL_MV | MADCTL_BGR);
		_width = ILI9341_TFTHEIGHT;
		_height = ILI9341_TFTWIDTH;
		break;
	case 2:
		m = (MADCTL_MY | MADCTL_BGR);
		_width = ILI9341_TFTWIDTH;
		_height = ILI9341_TFTHEIGHT;
		break;
	case 3:
		m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		_width = ILI9341_TFTHEIGHT;
		_height = ILI9341_TFTWIDTH;
		break;
	}

	sendCommand(ILI9341_MADCTL, &m, 1);
}

void ILI9341_TypeDef::InvertDisplay(uint8_t invert) {
	sendCommand(invert ? ILI9341_INVON : ILI9341_INVOFF);
}

void ILI9341_TypeDef::Scroll(uint16_t y) {
	uint8_t data[2];
	data[0] = y >> 8;
	data[1] = y & 0xff;
	sendCommand(ILI9341_VSCRSADD, (uint8_t *)data, 2);
}

void ILI9341_TypeDef::SetScrollMargins(uint16_t top, uint16_t bottom) {
	// TFA+VSA+BFA must equal 320
	if (top + bottom <= ILI9341_TFTHEIGHT) {
		uint16_t middle = ILI9341_TFTHEIGHT - (top + bottom);
		uint8_t data[6];
		data[0] = top >> 8;
		data[1] = top & 0xff;
		data[2] = middle >> 8;
		data[3] = middle & 0xff;
		data[4] = bottom >> 8;
		data[5] = bottom & 0xff;
		sendCommand(ILI9341_VSCRDEF, (uint8_t *)data, 6);
	}
}

void ILI9341_TypeDef::SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
	uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
	sendCommand(ILI9341_CASET); // Column address set
	LL_SPI_TransmitData8(spi, x1 >> 8);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, x1 & 0xFF);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, x2 >> 8);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, x2 & 0xFF);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	sendCommand(ILI9341_PASET); // Row address set
	LL_SPI_TransmitData8(spi, y1 >> 8);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, y1 & 0xFF);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, y2 >> 8);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, y2 & 0xFF);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	sendCommand(ILI9341_RAMWR); // Write to RAM
}

uint8_t ILI9341_TypeDef::ReadCommand(uint8_t commandByte, uint8_t index) {
	uint8_t data = 0x10 + index;
	sendCommand(0xD9, &data, 1); // Set Index Register
	sendCommand(commandByte);
	return LL_SPI_ReceiveData8(spi);
}