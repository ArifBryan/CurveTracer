#include "INA226.h"
#include <string.h>

void INA226_TypeDef::Init(uint8_t vShuntConvTime, uint8_t vBusConvTime, uint8_t avg) {
	uint16_t cfg;
	
	cfg = INA226_CONFIG_MODE_VSHVBUSCONT;
	cfg |= vShuntConvTime << 3;
	cfg |= vBusConvTime << 6;
	cfg |= avg << 9;
	
	Write(INA226_CONFIG, cfg);
}

void INA226_TypeDef::SetCurrentCal(float currentCal) {
	this->currentCal = currentCal;
}

void INA226_TypeDef::I2C_TransComplete_Handler() {
	if (readCount == 2) {
		vbus = (dataBuffer[0] << 8) | dataBuffer[1];
		regBuffer = INA226_VSHUNT;
		i2c->Read(addr, (uint8_t*)&regBuffer, 1, (uint8_t*)dataBuffer, 2);
		readCount = 1;
	}
	else if (readCount == 1) {
		vshunt = (dataBuffer[0] << 8) | dataBuffer[1];
		readCount = 0;
		readDone = 1;
	}
}

void INA226_TypeDef::ReadData() {
	i2c->WaitTransmission();
	if (interrupt) {
		readDone = 0;
		readCount = 2;
		regBuffer = INA226_VBUS;
		i2c->Read(addr, (uint8_t*)&regBuffer, 1, (uint8_t*)dataBuffer, 2);
	}
	else {
		vbus = Read(INA226_VBUS);
		vshunt = Read(INA226_VSHUNT);
	}
}

uint8_t INA226_TypeDef::IsReadComplete() {
	if (interrupt) {
		return readDone;		
	}
	return 1;
}

float INA226_TypeDef::GetVoltage() {
	return vbus * 1.25;
}

float INA226_TypeDef::GetCurrent() {
	return vshunt * 0.0025 / currentCal;
}

void INA226_TypeDef::Write(uint8_t reg, uint8_t *data, uint8_t len) {
	uint8_t tmp[len + 1];
	tmp[0] = reg;
	memcpy(tmp + 1, data, len);
	i2c->Write(addr, tmp, len + 1);
	i2c->WaitTransmission();
}

void INA226_TypeDef::Write(uint8_t reg, uint16_t data) {
	uint8_t tmp[3];
	tmp[0] = reg;
	tmp[1] = data >> 8;
	tmp[2] = data;
	i2c->Write(addr, tmp, 3);
	i2c->WaitTransmission();
}

uint16_t INA226_TypeDef::Read(uint8_t reg) {
	uint8_t rxData[2];
	i2c->Read(addr, &reg, 1, rxData, 2);
	i2c->WaitTransmission();
	
	return (rxData[0] << 8) | rxData[1];
}