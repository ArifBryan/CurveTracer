#include "ILI9341.h"
#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_spi.h>
#include <stm32f1xx_ll_utils.h>

static const uint8_t initCmd[] = { 
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
	SendCommand(ILI9341_SWRESET);
	LL_mDelay(150);
	
	const uint8_t *cmdPtr = initCmd;
	uint8_t cmd;
	while ((cmd = *cmdPtr++) > 0) {
		uint8_t x = *cmdPtr++;
		uint8_t cmdLen = x & 0x7F;
		SendCommand(cmd, (uint8_t*)cmdPtr, cmdLen);
		cmdPtr += cmdLen;
		if (x & 0x80) {LL_mDelay(150);}
	}
}

void ILI9341_TypeDef::SendCommand(uint8_t cmd, uint8_t *data, uint8_t dataLen) {
	LL_GPIO_ResetOutputPin(csGPIO, csPIN);
	
	LL_GPIO_ResetOutputPin(dcGPIO, dcPIN);
	LL_SPI_TransmitData8(spi, cmd);
	while (!LL_SPI_IsActiveFlag_RXNE(spi)) ;
	LL_SPI_ReceiveData8(spi);
	
	LL_GPIO_SetOutputPin(dcGPIO, dcPIN);
	for (uint8_t i = 0; i < dataLen; i++) {
		LL_SPI_TransmitData8(spi, data[i]);
		while (!LL_SPI_IsActiveFlag_RXNE(spi)) ;
		LL_SPI_ReceiveData8(spi);
	}
	LL_GPIO_SetOutputPin(csGPIO, csPIN);
}
