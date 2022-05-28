#pragma once

#include "I2CHandle.h"
#include <stm32f1xx_ll_dma.h>

#define INA226_CONFIG	0x00
#define INA226_VSHUNT	0x01
#define INA226_VBUS		0x02
#define INA226_POWER	0x03
#define INA226_CURRENT	0x04
#define INA226_CALIB	0x05
#define INA226_MASK		0x06
#define INA226_LIMIT	0x07
#define INA226_MFCID	0xFE
#define INA226_DIEID	0xFF

#define INA226_CONFIG_RESET	1 << 15

#define INA226_CONFIG_AVG_1		0b000
#define INA226_CONFIG_AVG_4		0b001
#define INA226_CONFIG_AVG_16	0b010
#define INA226_CONFIG_AVG_64	0b011
#define INA226_CONFIG_AVG_128	0b100
#define INA226_CONFIG_AVG_256	0b101
#define INA226_CONFIG_AVG_512	0b110
#define INA226_CONFIG_AVG_1024	0b111

#define INA226_CONFIG_VBUSCT_140us	0b000
#define INA226_CONFIG_VBUSCT_204us	0b001
#define INA226_CONFIG_VBUSCT_332us	0b010
#define INA226_CONFIG_VBUSCT_588us	0b011
#define INA226_CONFIG_VBUSCT_1100us	0b100
#define INA226_CONFIG_VBUSCT_2116us	0b101
#define INA226_CONFIG_VBUSCT_4156us	0b110
#define INA226_CONFIG_VBUSCT_8244us	0b111

#define INA226_CONFIG_VSHCT_140us	0b000
#define INA226_CONFIG_VSHCT_204us	0b001
#define INA226_CONFIG_VSHCT_332us	0b010
#define INA226_CONFIG_VSHCT_588us	0b011
#define INA226_CONFIG_VSHCT_1100us	0b100
#define INA226_CONFIG_VSHCT_2116us	0b101
#define INA226_CONFIG_VSHCT_4156us	0b110
#define INA226_CONFIG_VSHCT_8244us	0b111

#define INA226_CONFIG_MODE_POWERDOWN	0b000
#define INA226_CONFIG_MODE_VSHTRIG		0b001
#define INA226_CONFIG_MODE_VBUSTRIG		0b010
#define INA226_CONFIG_MODE_VSHVBUSTRIG	0b011
#define INA226_CONFIG_MODE_VSHCONT		0b101
#define INA226_CONFIG_MODE_VBUSCONT		0b110
#define INA226_CONFIG_MODE_VSHVBUSCONT	0b111

#define INA226_MASKbit_SOL	15U
#define INA226_MASKbit_SUL	14U
#define INA226_MASKbit_BOL	13U
#define INA226_MASKbit_BUL	12U
#define INA226_MASKbit_POL	11U
#define INA226_MASKbit_CNVR	10U
#define INA226_MASKbit_AFF	4U
#define INA226_MASKbit_CVRF	3U
#define INA226_MASKbit_OVF	2U
#define INA226_MASKbit_APOL	1U
#define INA226_MASKbit_LEN	0U

struct INA226_TypeDef {
	INA226_TypeDef(I2CHandleTypeDef *i2c, uint8_t addr, DMA_TypeDef *dma = 0, uint32_t dmaRxCh = 0, uint32_t dmaTxCh = 0) {
		this->i2c = i2c;
		this->addr = addr << 1;
		this->dma = dma;
		this->dmaRxCh = dmaRxCh;
		this->dmaTxCh = dmaTxCh;
		i2cPeriph = i2c->GetI2CPeriph();
	}
	void Init(void);
	void Write(uint8_t reg, uint8_t *data, uint8_t len);
	void Write(uint8_t reg, uint16_t data);
	void ReadData(void);
	float GetVoltage(void);
	float GetCurrent(void);
	void DMARx_IRQ_Handler(void);
	uint16_t Read(uint8_t reg);
private:
	I2CHandleTypeDef *i2c;
	I2C_TypeDef *i2cPeriph;
	uint8_t addr;
	DMA_TypeDef *dma;
	uint32_t dmaRxCh;
	uint32_t dmaTxCh;
	uint16_t currentCal = 1;
	volatile uint8_t dmaBusy;
	volatile uint8_t dmaBuffer[4];
	volatile uint16_t vbus;
	volatile uint16_t vshunt;
};