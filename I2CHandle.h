#ifndef _I2CHandle_LL_H
#define _I2CHandle_LL_H

#include <stm32f1xx_ll_i2c.h>

struct I2CHandleTypeDef {
public:
	I2CHandleTypeDef(I2C_TypeDef *i2c) {
		this->i2c = i2c;
	}
	void Init(LL_I2C_InitTypeDef *I2CInitStruct, void(*TransCompleteCallback)(void) = 0) {
		this->TransCompleteCallback = TransCompleteCallback;
		LL_I2C_EnableClockStretching(i2c);
		LL_I2C_Init(i2c, I2CInitStruct);
		// Disable Bit POS option when using interrupt
		// (except using special interrupt handler)
		if (!TransCompleteCallback) {
			LL_I2C_EnableBitPOS(i2c);
		}
		LL_I2C_Enable(i2c);
	}
	inline void GenerateStart() {
		LL_I2C_ClearFlag_ARLO(i2c);
		LL_I2C_ClearFlag_BERR(i2c);
		LL_I2C_ClearFlag_OVR(i2c);
		LL_I2C_ClearFlag_AF(i2c);
		LL_I2C_GenerateStartCondition(i2c);
	}
	inline void GenerateStop() {LL_I2C_GenerateStopCondition(i2c);}
	inline void Transmit(uint8_t data) {LL_I2C_TransmitData8(i2c, data);}
	inline uint8_t Receive() {return (LL_I2C_ReceiveData8(i2c));}
	inline uint8_t IsStartCondition() {return (LL_I2C_IsActiveFlag_SB(i2c));}
	inline uint8_t IsTransmmisionComplete() {
		uint8_t r = LL_I2C_IsActiveFlag_BTF(i2c);
		if (LL_I2C_IsActiveFlag_ADDR(i2c)) {
			LL_I2C_ClearFlag_ADDR(i2c);
			r = 1;
		}
		r |= LL_I2C_IsActiveFlag_AF(i2c);
		r |= LL_I2C_IsActiveFlag_RXNE(i2c);
		return (r);
	}
	inline uint8_t IsACK() {
		uint8_t ack = !LL_I2C_IsActiveFlag_AF(i2c);
		LL_I2C_ClearFlag_AF(i2c);
		return (ack);
	}
	inline void SetACK(uint8_t ack) {LL_I2C_AcknowledgeNextData(i2c, (ack ? LL_I2C_ACK : LL_I2C_NACK));}
	inline uint8_t IsBusy() {return (LL_I2C_IsActiveFlag_BUSY(i2c));}
	uint8_t Read(uint8_t addr, uint8_t *reg, uint8_t regLen, uint8_t *rxData, uint16_t dataLen, uint16_t tout = 100);
	uint8_t Write(uint8_t addr, uint8_t *data, uint16_t dataLen, uint16_t tout = 100);
	void I2C_EV_IRQ_Handler(void);
	uint8_t WaitTransmission(uint16_t tout = 1000);
	I2C_TypeDef* GetI2CPeriph() {return i2c;}
private:
	I2C_TypeDef *i2c;
	void (*TransCompleteCallback)(void);
	volatile uint8_t state;
	volatile uint8_t *txData;
	volatile uint8_t *rxData;
	volatile uint16_t txLen;
	volatile uint16_t rxLen;
	volatile uint8_t addr;
};

#endif // !_I2CPeriph_H
