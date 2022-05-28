#include "INA226.h"
#include <string.h>

void INA226_TypeDef::Init() {
	uint16_t cfg;
	
	cfg = INA226_CONFIG_MODE_VSHVBUSCONT;
	cfg |= INA226_CONFIG_VSHCT_204us << 3;
	cfg |= INA226_CONFIG_VBUSCT_204us << 6;
	cfg |= INA226_CONFIG_AVG_16 << 9;
	
	Write(INA226_CONFIG, cfg);
}

void INA226_TypeDef::DMARx_IRQ_Handler() {
	if (dmaBusy) {
		i2c->GenerateStop();
		vbus = (dmaBuffer[0] << 8) | dmaBuffer[1];
		//vbus = (dmaBuffer[2] << 8) | dmaBuffer[3];
		dmaBusy = 0;
		LL_DMA_DisableIT_TC(dma, dmaRxCh);
	}
}

void INA226_TypeDef::ReadData() {
	while (i2c->IsBusy()) ;
	if (dmaRxCh) {
		if (dmaBusy == 0) {
			dmaBusy = 1;
			LL_DMA_DisableChannel(dma, dmaRxCh);
			LL_DMA_SetMemoryAddress(dma, dmaRxCh, (uint32_t)dmaBuffer);
			LL_DMA_SetDataLength(dma, dmaRxCh, 4);
			LL_DMA_EnableChannel(dma, dmaRxCh);
			LL_DMA_EnableIT_TC(dma, dmaRxCh);
			i2c->DMALastTransfer(1);
			i2c->Read(addr, (uint8_t*)INA226_VBUS, 1, _NULL, 0);
		}
	}
	else {
		vbus = Read(INA226_VBUS);
		vshunt = Read(INA226_VSHUNT);
	}
}

float INA226_TypeDef::GetVoltage() {
	return vbus * 1.25;
}

float INA226_TypeDef::GetCurrent() {
	return vshunt * currentCal;
}

void INA226_TypeDef::Write(uint8_t reg, uint8_t *data, uint8_t len) {
	uint8_t tmp[len + 1];
	tmp[0] = reg;
	memcpy(tmp + 1, data, len);
	i2c->Write(addr, tmp, len + 1);
}

void INA226_TypeDef::Write(uint8_t reg, uint16_t data) {
	uint8_t tmp[3];
	tmp[0] = reg;
	tmp[1] = data >> 8;
	tmp[2] = data;
	i2c->Write(addr, tmp, 3);
}

uint16_t INA226_TypeDef::Read(uint8_t reg) {
	uint8_t rxData[2];
	i2c->Read(addr, &reg, 1, rxData, 2);
	return (rxData[0] << 8) | rxData[1];
}