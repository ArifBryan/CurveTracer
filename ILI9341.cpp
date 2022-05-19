#include "ILI9341.h"
#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_spi.h>
#include <stm32f1xx_ll_utils.h>

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

void ILI9341_TypeDef::writePixel(int16_t x, int16_t y, uint16_t color) {
	if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
		setAddrWindow(x, y, 1, 1);
		SPI_WRITE16(color);
	}
}

void ILI9341_TypeDef::swapBytes(uint16_t *src, uint32_t len, uint16_t *dest) {
	if (!dest)
		dest = src; // NULL -> overwrite src buffer
	for (uint32_t i = 0; i < len; i++) {
		dest[i] = __builtin_bswap16(src[i]);
	}
}

void ILI9341_TypeDef::writePixels(uint16_t *colors,
	uint32_t len,
	bool block,
	bool bigEndian) {

	if (!len)
		return; // Avoid 0-byte transfers

	  // avoid paramater-not-used complaints
	(void)block;
	(void)bigEndian;

	// All other cases (bitbang SPI or non-DMA hard SPI or parallel),
	// use a loop with the normal 16-bit data write function:

	if (!bigEndian) {
		while (len--) {
			SPI_WRITE16(*colors++);
		}
	}
	else {
		// Well this is awkward. SPI_WRITE16() was designed for little-endian
		// hosts and big-endian displays as that's nearly always the typical
		// case. If the bigEndian flag was set, data is already in display's
		// order...so each pixel needs byte-swapping before being issued.
		// Rather than having a separate big-endian SPI_WRITE16 (adding more
		// bloat), it's preferred if calling function is smart and only uses
		// bigEndian where DMA is supported. But we gotta handle this...
		while (len--) {
			SPI_WRITE16(__builtin_bswap16(*colors++));
		}
	}
}

void ILI9341_TypeDef::writeColor(uint16_t color, uint32_t len) {
	if (!len)
		return; // Avoid 0-byte transfers

	while (len--) {
		SPI_WRITE16(color);
	}
}

void ILI9341_TypeDef::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	if (w && h) {
		// Nonzero width and height?
		if (w < 0) {
			// If negative width...
			x += w + 1; //   Move X to left edge
			w = -w; //   Use positive width
		}
		if (x < _width) {
			// Not off right
			if (h < 0) {
				// If negative height...
				y += h + 1; //   Move Y to top edge
				h = -h; //   Use positive height
			}
			if (y < _height) {
				// Not off bottom
				int16_t x2 = x + w - 1;
				if (x2 >= 0) {
					// Not off left
					int16_t y2 = y + h - 1;
					if (y2 >= 0) {
						// Not off top
					  // Rectangle partly or fully overlaps screen
						if (x < 0) {
							x = 0;
							w = x2 + 1;
						} // Clip left
						if (y < 0) {
							y = 0;
							h = y2 + 1;
						} // Clip top
						if (x2 >= _width) {
							w = _width - x;
						} // Clip right
						if (y2 >= _height) {
							h = _height - y;
						} // Clip bottom
						writeFillRectPreclipped(x, y, w, h, color);
					}
				}
			}
		}
	}
}

void inline ILI9341_TypeDef::writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	if ((y >= 0) && (y < _height) && w) {
		// Y on screen, nonzero width
		if (w < 0) {
			// If negative width...
			x += w + 1; //   Move X to left edge
			w = -w; //   Use positive width
		}
		if (x < _width) {
			// Not off right
			int16_t x2 = x + w - 1;
			if (x2 >= 0) {
				// Not off left
			  // Line partly or fully overlaps screen
				if (x < 0) {
					x = 0;
					w = x2 + 1;
				} // Clip left
				if (x2 >= _width) {
					w = _width - x;
				} // Clip right
				writeFillRectPreclipped(x, y, w, 1, color);
			}
		}
	}
}

void inline ILI9341_TypeDef::writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	if ((x >= 0) && (x < _width) && h) {
		// X on screen, nonzero height
		if (h < 0) {
			// If negative height...
			y += h + 1; //   Move Y to top edge
			h = -h; //   Use positive height
		}
		if (y < _height) {
			// Not off bottom
			int16_t y2 = y + h - 1;
			if (y2 >= 0) {
				// Not off top
			  // Line partly or fully overlaps screen
				if (y < 0) {
					y = 0;
					h = y2 + 1;
				} // Clip top
				if (y2 >= _height) {
					h = _height - y;
				} // Clip bottom
				writeFillRectPreclipped(x, y, 1, h, color);
			}
		}
	}
}

inline void ILI9341_TypeDef::writeFillRectPreclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	setAddrWindow(x, y, w, h);
	writeColor(color, (uint32_t)w * h);
}

void ILI9341_TypeDef::drawPixel(int16_t x, int16_t y, uint16_t color) {
	// Clip first...
	if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
		// THEN set up transaction (if needed) and draw...
		startWrite();
		setAddrWindow(x, y, 1, 1);
		SPI_WRITE16(color);
		endWrite();
	}
}

void ILI9341_TypeDef::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	if (w && h) {
		// Nonzero width and height?
		if (w < 0) {
			// If negative width...
			x += w + 1; //   Move X to left edge
			w = -w; //   Use positive width
		}
		if (x < _width) {
			// Not off right
			if (h < 0) {
				// If negative height...
				y += h + 1; //   Move Y to top edge
				h = -h; //   Use positive height
			}
			if (y < _height) {
				// Not off bottom
				int16_t x2 = x + w - 1;
				if (x2 >= 0) {
					// Not off left
					int16_t y2 = y + h - 1;
					if (y2 >= 0) {
						// Not off top
					  // Rectangle partly or fully overlaps screen
						if (x < 0) {
							x = 0;
							w = x2 + 1;
						} // Clip left
						if (y < 0) {
							y = 0;
							h = y2 + 1;
						} // Clip top
						if (x2 >= _width) {
							w = _width - x;
						} // Clip right
						if (y2 >= _height) {
							h = _height - y;
						} // Clip bottom
						startWrite();
						writeFillRectPreclipped(x, y, w, h, color);
						endWrite();
					}
				}
			}
		}
	}
}

void ILI9341_TypeDef::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	if ((y >= 0) && (y < _height) && w) {
		// Y on screen, nonzero width
		if (w < 0) {
			// If negative width...
			x += w + 1; //   Move X to left edge
			w = -w; //   Use positive width
		}
		if (x < _width) {
			// Not off right
			int16_t x2 = x + w - 1;
			if (x2 >= 0) {
				// Not off left
			  // Line partly or fully overlaps screen
				if (x < 0) {
					x = 0;
					w = x2 + 1;
				} // Clip left
				if (x2 >= _width) {
					w = _width - x;
				} // Clip right
				startWrite();
				writeFillRectPreclipped(x, y, w, 1, color);
				endWrite();
			}
		}
	}
}

void ILI9341_TypeDef::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	if ((x >= 0) && (x < _width) && h) {
		// X on screen, nonzero height
		if (h < 0) {
			// If negative height...
			y += h + 1; //   Move Y to top edge
			h = -h; //   Use positive height
		}
		if (y < _height) {
			// Not off bottom
			int16_t y2 = y + h - 1;
			if (y2 >= 0) {
				// Not off top
			  // Line partly or fully overlaps screen
				if (y < 0) {
					y = 0;
					h = y2 + 1;
				} // Clip top
				if (y2 >= _height) {
					h = _height - y;
				} // Clip bottom
				startWrite();
				writeFillRectPreclipped(x, y, 1, h, color);
				endWrite();
			}
		}
	}
}

void ILI9341_TypeDef::pushColor(uint16_t color) {
	startWrite();
	SPI_WRITE16(color);
	endWrite();
}

void ILI9341_TypeDef::drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors, int16_t w, int16_t h) {
	int16_t x2, y2; // Lower-right coord
	if ((x >= _width) ||            // Off-edge right
	    (y >= _height) ||           // " top
	    ((x2 = (x + w - 1)) < 0) || // " left
	    ((y2 = (y + h - 1)) < 0))
		return; // " bottom

	int16_t bx1 = 0, by1 = 0, // Clipped top-left within bitmap
	    saveW = w; // Save original bitmap width value
	if (x < 0) {
		// Clip left
		w += x;
		bx1 = -x;
		x = 0;
	}
	if (y < 0) {
		// Clip top
		h += y;
		by1 = -y;
		y = 0;
	}
	if (x2 >= _width)
		w = _width - x; // Clip right
	if (y2 >= _height)
		h = _height - y; // Clip bottom

	pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left
	startWrite();
	setAddrWindow(x, y, w, h); // Clipped area
	while (h--) {
		// For each (clipped) scanline...
		writePixels(pcolors, w); // Push one (clipped) row
		pcolors += saveW; // Advance pointer by one full (unclipped) line
	}
	endWrite();
}

////////////////////////////////////////

void ILI9341_TypeDef::startWrite() {
	SPI_CS_LOW();
}

void ILI9341_TypeDef::endWrite() {
	SPI_CS_HIGH();
}

void ILI9341_TypeDef::SPI_WRITE16(uint16_t data) {
	LL_SPI_TransmitData8(spi, data >> 8);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, data);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
}

void ILI9341_TypeDef::SPI_WRITE32(uint32_t data) {
	LL_SPI_TransmitData8(spi, data >> 24);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, data >> 16);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, data >> 8);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	LL_SPI_TransmitData8(spi, data);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
}

void ILI9341_TypeDef::invertDisplay(uint8_t invert) {
	sendCommand(invert ? ILI9341_INVON : ILI9341_INVOFF);
}

uint16_t ILI9341_TypeDef::color565(uint8_t red, uint8_t green, uint8_t blue) {
	return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

void ILI9341_TypeDef::spiWrite(uint8_t b) {
	LL_SPI_TransmitData8(spi, b);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
}

void ILI9341_TypeDef::writeCommand(uint8_t cmd) {
	SPI_DC_LOW();
	spiWrite(cmd);
	SPI_DC_HIGH();
}

uint8_t ILI9341_TypeDef::spiRead(void) {
	LL_SPI_TransmitData8(spi, 0);
	while (LL_SPI_IsActiveFlag_BSY(spi)) ;
	return LL_SPI_ReceiveData8(spi);	
}

void ILI9341_TypeDef::write16(uint16_t w) {
	SPI_WRITE16(w);
}

void ILI9341_TypeDef::writeCommand16(uint16_t cmd) {
	SPI_DC_LOW();
	write16(cmd);
	SPI_DC_HIGH();
}

uint16_t ILI9341_TypeDef::read16(void) {
	uint16_t data = spiRead() << 8;
	data |= spiRead();
	
	return data;
}

/////////////////////////////////////////////////////

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
}

void ILI9341_TypeDef::setRotation(uint8_t m) {
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

void ILI9341_TypeDef::scrollTo(uint16_t y) {
	uint8_t data[2];
	data[0] = y >> 8;
	data[1] = y & 0xff;
	sendCommand(ILI9341_VSCRSADD, (uint8_t *)data, 2);
}

void ILI9341_TypeDef::setScrollMargins(uint16_t top, uint16_t bottom) {
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

void ILI9341_TypeDef::setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
	uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);
	writeCommand(ILI9341_CASET); // Column address set
	SPI_WRITE16(x1);
	SPI_WRITE16(x2);
	writeCommand(ILI9341_PASET); // Row address set
	SPI_WRITE16(y1);
	SPI_WRITE16(y2);
	writeCommand(ILI9341_RAMWR); // Write to RAM
}

uint8_t ILI9341_TypeDef::readCommand(uint8_t commandByte, uint8_t index) {
	uint8_t data = 0x10 + index;
	sendCommand(0xD9, &data, 1); // Set Index Register
	sendCommand(commandByte);
	return LL_SPI_ReceiveData8(spi);
}