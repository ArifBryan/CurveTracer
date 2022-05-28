#ifndef _I2CHandle_LL_H
#define _I2CHandle_LL_H

#include <stm32f1xx_ll_i2c.h>

struct I2CHandleTypeDef {
public:
	I2CHandleTypeDef(I2C_TypeDef *i2cDev) {i2cdev = i2cDev;}
	void Init(LL_I2C_InitTypeDef *I2CInitStruct) {
		LL_I2C_EnableClockStretching(i2cdev);
		LL_I2C_Init(i2cdev, I2CInitStruct);
		LL_I2C_EnableBitPOS(i2cdev);
		LL_I2C_Enable(i2cdev);
	}
	inline void GenerateStart() {
		LL_I2C_ClearFlag_ARLO(i2cdev);
		LL_I2C_ClearFlag_BERR(i2cdev);
		LL_I2C_ClearFlag_OVR(i2cdev);
		LL_I2C_ClearFlag_AF(i2cdev);
		LL_I2C_GenerateStartCondition(i2cdev);
	}
	inline void GenerateStop() {LL_I2C_GenerateStopCondition(i2cdev);}
	inline void Transmit(uint8_t data) {LL_I2C_TransmitData8(i2cdev, data);}
	inline uint8_t Receive() {return (LL_I2C_ReceiveData8(i2cdev));}
	inline uint8_t IsStartCondition() {return (LL_I2C_IsActiveFlag_SB(i2cdev));}
	inline uint8_t IsTransmmisionComplete() {
		uint8_t r = LL_I2C_IsActiveFlag_BTF(i2cdev);
		if (LL_I2C_IsActiveFlag_ADDR(i2cdev)) {
			LL_I2C_ClearFlag_ADDR(i2cdev);
			r = 1;
		}
		r |= LL_I2C_IsActiveFlag_AF(i2cdev);
		r |= LL_I2C_IsActiveFlag_RXNE(i2cdev);
		return (r);
	}
	inline uint8_t IsACK() {
		uint8_t ack = !LL_I2C_IsActiveFlag_AF(i2cdev);
		LL_I2C_ClearFlag_AF(i2cdev);
		return (ack);
	}
	inline void SetACK(uint8_t ack) {LL_I2C_AcknowledgeNextData(i2cdev, (ack ? LL_I2C_ACK : LL_I2C_NACK));}
	inline uint8_t IsBusy() {return (LL_I2C_IsActiveFlag_BUSY(i2cdev));}
	inline void InterruptBuffer(uint8_t enable) {
		if (enable){LL_I2C_EnableIT_BUF(i2cdev); }
		else {LL_I2C_DisableIT_BUF(i2cdev);}
	}
	inline void InterruptTXEmpty(uint8_t enable) {
		if (enable) {LL_I2C_EnableIT_TX(i2cdev); }
		else {LL_I2C_DisableIT_TX(i2cdev); }
	}
	inline void InterruptRXNotEmpty(uint8_t enable) {
		if (enable) {LL_I2C_EnableIT_RX(i2cdev); }
		else {LL_I2C_DisableIT_RX(i2cdev); }
	}
	inline void InterruptEvent(uint8_t enable) {
		if (enable) {LL_I2C_EnableIT_EVT(i2cdev); }
		else {LL_I2C_DisableIT_EVT(i2cdev); }
	}
	inline void InterruptError(uint8_t enable) {
		if (enable) {LL_I2C_EnableIT_ERR(i2cdev); }
		else {LL_I2C_DisableIT_ERR(i2cdev);}
	}
	inline void DMARequestTX(uint8_t enable) {
		if (enable) {LL_I2C_EnableDMAReq_TX(i2cdev); }
		else {LL_I2C_DisableDMAReq_TX(i2cdev);}
	}
	inline void DMARequestRX(uint8_t enable) {
		if (enable) {LL_I2C_EnableDMAReq_RX(i2cdev); }
		else {LL_I2C_DisableDMAReq_RX(i2cdev); }
	}
	inline void DMALastTransfer(uint8_t enable) {
		if (enable) {LL_I2C_EnableLastDMA(i2cdev); }
		else {LL_I2C_DisableLastDMA(i2cdev); }
	}
	inline void OwnAddress2(uint8_t enable) {
		if (enable) {LL_I2C_EnableOwnAddress2(i2cdev); }
		else {LL_I2C_DisableOwnAddress2(i2cdev); }
	}
	uint8_t Read(uint8_t addr, uint8_t reg[], uint8_t regLen, uint8_t data[], uint16_t dataLen, uint16_t tout = 100);
	uint8_t Write(uint8_t addr, uint8_t data[], uint16_t dataLen, uint16_t tout = 100);
	I2C_TypeDef* GetI2CPeriph() {return i2cdev;}
private:
	I2C_TypeDef *i2cdev;
};

#endif // !_I2CPeriph_H
