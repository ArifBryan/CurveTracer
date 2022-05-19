#pragma once

#include <stm32f1xx.h>
#include <stm32f1xx_ll_gpio.h>
#include "LL_GFX.h"

#define ILI9341_TFTWIDTH 240  ///< ILI9341 max TFT width
#define ILI9341_TFTHEIGHT 320 ///< ILI9341 max TFT height

#define ILI9341_NOP 0x00     ///< No-op register
#define ILI9341_SWRESET 0x01 ///< Software reset register
#define ILI9341_RDDID 0x04   ///< Read display identification information
#define ILI9341_RDDST 0x09   ///< Read Display Status

#define ILI9341_SLPIN 0x10  ///< Enter Sleep Mode
#define ILI9341_SLPOUT 0x11 ///< Sleep Out
#define ILI9341_PTLON 0x12  ///< Partial Mode ON
#define ILI9341_NORON 0x13  ///< Normal Display Mode ON

#define ILI9341_RDMODE 0x0A     ///< Read Display Power Mode
#define ILI9341_RDMADCTL 0x0B   ///< Read Display MADCTL
#define ILI9341_RDPIXFMT 0x0C   ///< Read Display Pixel Format
#define ILI9341_RDIMGFMT 0x0D   ///< Read Display Image Format
#define ILI9341_RDSELFDIAG 0x0F ///< Read Display Self-Diagnostic Result

#define ILI9341_INVOFF 0x20   ///< Display Inversion OFF
#define ILI9341_INVON 0x21    ///< Display Inversion ON
#define ILI9341_GAMMASET 0x26 ///< Gamma Set
#define ILI9341_DISPOFF 0x28  ///< Display OFF
#define ILI9341_DISPON 0x29   ///< Display ON

#define ILI9341_CASET 0x2A ///< Column Address Set
#define ILI9341_PASET 0x2B ///< Page Address Set
#define ILI9341_RAMWR 0x2C ///< Memory Write
#define ILI9341_RAMRD 0x2E ///< Memory Read

#define ILI9341_PTLAR 0x30    ///< Partial Area
#define ILI9341_VSCRDEF 0x33  ///< Vertical Scrolling Definition
#define ILI9341_MADCTL 0x36   ///< Memory Access Control
#define ILI9341_VSCRSADD 0x37 ///< Vertical Scrolling Start Address
#define ILI9341_PIXFMT 0x3A   ///< COLMOD: Pixel Format Set

#define ILI9341_FRMCTR1                                                        \
0xB1 ///< Frame Rate Control (In Normal Mode/Full Colors)
#define ILI9341_FRMCTR2 0xB2 ///< Frame Rate Control (In Idle Mode/8 colors)
#define ILI9341_FRMCTR3                                                        \
  0xB3 ///< Frame Rate control (In Partial Mode/Full Colors)
#define ILI9341_INVCTR 0xB4  ///< Display Inversion Control
#define ILI9341_DFUNCTR 0xB6 ///< Display Function Control

#define ILI9341_PWCTR1 0xC0 ///< Power Control 1
#define ILI9341_PWCTR2 0xC1 ///< Power Control 2
#define ILI9341_PWCTR3 0xC2 ///< Power Control 3
#define ILI9341_PWCTR4 0xC3 ///< Power Control 4
#define ILI9341_PWCTR5 0xC4 ///< Power Control 5
#define ILI9341_VMCTR1 0xC5 ///< VCOM Control 1
#define ILI9341_VMCTR2 0xC7 ///< VCOM Control 2

#define ILI9341_RDID1 0xDA ///< Read ID 1
#define ILI9341_RDID2 0xDB ///< Read ID 2
#define ILI9341_RDID3 0xDC ///< Read ID 3
#define ILI9341_RDID4 0xDD ///< Read ID 4

#define ILI9341_GMCTRP1 0xE0 ///< Positive Gamma Correction
#define ILI9341_GMCTRN1 0xE1 ///< Negative Gamma Correction
//#define ILI9341_PWCTR6     0xFC

// Color definitions
#define ILI9341_BLACK 0x0000       ///<   0,   0,   0
#define ILI9341_NAVY 0x000F        ///<   0,   0, 123
#define ILI9341_DARKGREEN 0x03E0   ///<   0, 125,   0
#define ILI9341_DARKCYAN 0x03EF    ///<   0, 125, 123
#define ILI9341_MAROON 0x7800      ///< 123,   0,   0
#define ILI9341_PURPLE 0x780F      ///< 123,   0, 123
#define ILI9341_OLIVE 0x7BE0       ///< 123, 125,   0
#define ILI9341_LIGHTGREY 0xC618   ///< 198, 195, 198
#define ILI9341_DARKGREY 0x7BEF    ///< 123, 125, 123
#define ILI9341_BLUE 0x001F        ///<   0,   0, 255
#define ILI9341_GREEN 0x07E0       ///<   0, 255,   0
#define ILI9341_CYAN 0x07FF        ///<   0, 255, 255
#define ILI9341_RED 0xF800         ///< 255,   0,   0
#define ILI9341_MAGENTA 0xF81F     ///< 255,   0, 255
#define ILI9341_YELLOW 0xFFE0      ///< 255, 255,   0
#define ILI9341_WHITE 0xFFFF       ///< 255, 255, 255
#define ILI9341_ORANGE 0xFD20      ///< 255, 165,   0
#define ILI9341_GREENYELLOW 0xAFE5 ///< 173, 255,  41
#define ILI9341_PINK 0xFC18        ///< 255, 130, 198
	
class ILI9341_TypeDef : public Adafruit_GFX {
public:
	ILI9341_TypeDef(SPI_TypeDef *spi, GPIO_TypeDef *csGPIO, uint32_t csPIN, GPIO_TypeDef *dcGPIO, uint32_t dcPIN) 
		: Adafruit_GFX(ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT){
		this->spi = spi;
		this->csGPIO = csGPIO;
		this->dcGPIO = dcGPIO;
		this->csPIN = csPIN;
		this->dcPIN = dcPIN;
	}
	void Init(void);
	void setRotation(uint8_t r);
	void invertDisplay(uint8_t invert);
	void scrollTo(uint16_t y);
	void setScrollMargins(uint16_t top, uint16_t bottom);
	void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
	void startWrite(void);
	void endWrite(void);
	void sendCommand(uint8_t commandByte, uint8_t *dataBytes, uint8_t numDataBytes);
	void sendCommand(uint8_t commandByte) {
		sendCommand(commandByte, 0, 0);
	}
	void sendCommand16(uint16_t commandWord, const uint8_t *dataBytes = 0, uint8_t numDataBytes = 0);
	uint8_t readcommand8(uint8_t commandByte, uint8_t index = 0);
	uint16_t readcommand16(uint16_t addr);
	uint8_t readCommand(uint8_t reg, uint8_t index = 0);
	void writePixel(int16_t x, int16_t y, uint16_t color);
	void writePixels(uint16_t *colors, uint32_t len, bool block = true, bool bigEndian = false);
	void writeColor(uint16_t color, uint32_t len);
	void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	inline void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	// Another new function, companion to the new non-blocking
	// writePixels() variant.
	void dmaWait(void);
	// Used by writePixels() in some situations, but might have rare need in
	// user code, so it's public...
	bool dmaBusy(void) const; // true if DMA is used and busy, false otherwise
	void swapBytes(uint16_t *src, uint32_t len, uint16_t *dest = NULL);
	void drawPixel(int16_t x, int16_t y, uint16_t color);
	void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
	void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
	void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
	void pushColor(uint16_t color);
	using Adafruit_GFX::drawRGBBitmap; // Check base class first
	void drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors, int16_t w, int16_t h);

	uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

	// Despite parallel additions, function names kept for compatibility:
	void spiWrite(uint8_t b); // Write single byte as DATA
	void writeCommand(uint8_t cmd); // Write single byte as COMMAND
	uint8_t spiRead(void); // Read single byte of data
	void write16(uint16_t w); // Write 16-bit value as DATA
	void writeCommand16(uint16_t cmd); // Write 16-bit value as COMMAND
	uint16_t read16(void); 
	
	void SPI_WRITE16(uint16_t w); // Not inline
	void SPI_WRITE32(uint32_t l); // Not inline
	
	void SPI_CS_HIGH(void) {
		LL_GPIO_SetOutputPin(csGPIO, csPIN);
	}
	void SPI_CS_LOW(void) {
		LL_GPIO_ResetOutputPin(csGPIO, csPIN);
	}
	void SPI_DC_HIGH(void) {
		LL_GPIO_SetOutputPin(dcGPIO, dcPIN);
	}
	void SPI_DC_LOW(void) {
		LL_GPIO_ResetOutputPin(dcGPIO, dcPIN);
	}
private:
	SPI_TypeDef *spi;
	GPIO_TypeDef *csGPIO;
	GPIO_TypeDef *dcGPIO;
	uint32_t csPIN;
	uint32_t dcPIN;
};
