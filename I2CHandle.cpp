#include <stm32f1xx_ll_cortex.h>
#include "I2CHandle.h"

#define STATE_IDLE		0
#define STATE_START		1
#define STATE_REPSTART	2
#define STATE_ADDR		3
#define STATE_TX		4
#define STATE_RX		5
#define STATE_RX1		6
#define STATE_RX2		7
#define STATE_STOP		8
#define STATE_ERROR		9

uint8_t I2CHandleTypeDef::Read(uint8_t devAddr, uint8_t *regAddr, uint8_t regAddrLen, uint8_t *rxData, uint16_t dataLen, uint16_t tout) {
	uint8_t status = 0;
	
	while ((IsBusy() || state != STATE_IDLE) && tout > 0) {
		tout -= LL_SYSTICK_IsActiveCounterFlag();
	}
	
	if (TransCompleteCallback) {
		state = STATE_START;
		txData = regAddr;
		txLen = regAddrLen;
		this->rxData = rxData;
		rxLen = dataLen;
		addr = devAddr;
		LL_I2C_GenerateStartCondition(i2c);
		LL_I2C_EnableIT_EVT(i2c);
		status = 1;
	}
	else {
		//Start
		GenerateStart();
		while (!IsStartCondition() && tout > 0) {
			tout -= LL_SYSTICK_IsActiveCounterFlag();
		}
		//Address
		Transmit(devAddr);
		while (!IsTransmmisionComplete() && tout > 0) {
			tout -= LL_SYSTICK_IsActiveCounterFlag();
		}
		if (IsACK()) {
			status = 1;
			for (uint8_t i = 0; i < regAddrLen; i++) {
				//Register Address
				Transmit(regAddr[i]);
				while (!IsTransmmisionComplete() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				status &= IsACK();
			}
			//Start
			GenerateStart();
			while (!IsStartCondition() && tout > 0) {
				tout -= LL_SYSTICK_IsActiveCounterFlag();
			}
			if (dataLen == 1) {
				//Read Address
				Transmit(devAddr | 1);
				while (!IsTransmmisionComplete() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				SetACK(0);	
				GenerateStop();
				status &= IsACK();
				//Stop
				while (IsBusy() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				rxData[0] = Receive();
			}
			else if (dataLen == 2) {
				//Read Address
				SetACK(1);	
				Transmit(devAddr | 1);
				while (!IsTransmmisionComplete() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				SetACK(0);		
				status &= IsACK();
				while (!IsTransmmisionComplete() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				rxData[0] = Receive();
				//Stop
				GenerateStop();
				while (IsBusy() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				rxData[1] = Receive();			
			}
			else {
				uint16_t i;
				//Read Address
				SetACK(1);	
				Transmit(devAddr | 1);
				while (!IsTransmmisionComplete() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}		
				status &= IsACK();
				if (dataLen == 0) {return status;}
				dataLen -= 2;
				for (i = 0; i < dataLen; i++) {
					while (!IsTransmmisionComplete() && tout > 0) {
						tout -= LL_SYSTICK_IsActiveCounterFlag();
					}
					SetACK(i < (dataLen - 1));
					rxData[i] = Receive();
				}
				while (!IsTransmmisionComplete() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				rxData[i] = Receive();
				//Stop
				GenerateStop();
				while (IsBusy() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				rxData[i + 1] = Receive();
			}
		}
		else {
			//Stop
			GenerateStop();
			while (IsBusy() && tout > 0) {
				tout -= LL_SYSTICK_IsActiveCounterFlag();
			}
		}
	}
	return (status);
}

uint8_t I2CHandleTypeDef::Write(uint8_t devAddr, uint8_t *data, uint16_t dataLen, uint16_t tout) {
	uint8_t status = 0;
	
	while ((IsBusy() || state != STATE_IDLE) && tout > 0) {
		tout -= LL_SYSTICK_IsActiveCounterFlag();
	}
	
	if (TransCompleteCallback) {
		state = STATE_START;
		txData = data;
		txLen = dataLen;
		addr = devAddr;
		LL_I2C_GenerateStartCondition(i2c);
		LL_I2C_EnableIT_TX(i2c);
		LL_I2C_EnableIT_EVT(i2c);
		status = 1;
	}
	else {
		//Start
		GenerateStart();
		while (!IsStartCondition() && tout > 0) {
			tout -= LL_SYSTICK_IsActiveCounterFlag();
		}
		//Address
		Transmit(devAddr);
		while (!IsTransmmisionComplete() && tout > 0) {
			tout -= LL_SYSTICK_IsActiveCounterFlag();
		}
		if (IsACK()) {
			status = 1;
			//Data
			for (uint16_t i = 0; i < dataLen; i++) {
				Transmit(data[i]);
				while (!IsTransmmisionComplete() && tout > 0) {
					tout -= LL_SYSTICK_IsActiveCounterFlag();
				}
				status &= IsACK();
			}
		}
		//Stop
		GenerateStop();
		while (IsBusy() && tout > 0) {
			tout -= LL_SYSTICK_IsActiveCounterFlag();
		}
	}
		
	return (status && (tout > 0));
}

void I2CHandleTypeDef::I2C_EV_IRQ_Handler() {
	if (state != STATE_IDLE) {
		if (LL_I2C_IsActiveFlag_AF(i2c) || LL_I2C_IsActiveFlag_ARLO(i2c) || LL_I2C_IsActiveFlag_BERR(i2c)) {
			LL_I2C_ClearFlag_AF(i2c);
			LL_I2C_ClearFlag_ARLO(i2c);
			LL_I2C_ClearFlag_BERR(i2c);
			LL_I2C_GenerateStopCondition(i2c);
			state = STATE_ERROR;
		}
		switch (state) {
		case STATE_START:
			if (LL_I2C_IsActiveFlag_SB(i2c)) {
				LL_I2C_TransmitData8(i2c, addr | 0);
				LL_I2C_EnableIT_TX(i2c);
				state = STATE_ADDR;
			}
			break;
		case STATE_REPSTART:
			if (LL_I2C_IsActiveFlag_SB(i2c)) {
				LL_I2C_TransmitData8(i2c, addr | 1);
				LL_I2C_EnableIT_RX(i2c);
				state = STATE_ADDR;
			}
			break;
		case STATE_ADDR:
			if (LL_I2C_IsActiveFlag_ADDR(i2c)) {
				LL_I2C_ClearFlag_ADDR(i2c);
				if (txLen > 0) {
					state = STATE_TX;
				}
				else if (rxLen > 0) {
					if (rxLen == 1) {
						LL_I2C_AcknowledgeNextData(i2c, LL_I2C_NACK);
					}
					else {
						LL_I2C_AcknowledgeNextData(i2c, LL_I2C_ACK);
					}
					state = STATE_RX;
				}
				else {
					LL_I2C_GenerateStopCondition(i2c);
					state = STATE_STOP;
				}
			}
			break;
		case STATE_TX:
			if (LL_I2C_IsActiveFlag_TXE(i2c)) {
				if (txLen > 0) {
					txLen--;
					LL_I2C_TransmitData8(i2c, *txData);
					if (txLen > 0){txData++; }
				}
				else if (rxLen > 0) {
					LL_I2C_GenerateStartCondition(i2c);
					state = STATE_REPSTART;
				}
				else if (LL_I2C_IsActiveFlag_BTF(i2c)) {
					LL_I2C_GenerateStopCondition(i2c);
					state = STATE_STOP;
				}
			}
			break;
		case STATE_RX:
			if (LL_I2C_IsActiveFlag_RXNE(i2c)) {
				if (rxLen > 2) {
					rxLen--;
					*rxData = LL_I2C_ReceiveData8(i2c);
					rxData++;
				}
				else if (rxLen == 2) {
					rxLen--;
					*rxData = LL_I2C_ReceiveData8(i2c);
					rxData++;
					LL_I2C_AcknowledgeNextData(i2c, LL_I2C_NACK);
					LL_I2C_GenerateStopCondition(i2c);
				}
				else {
					*rxData = LL_I2C_ReceiveData8(i2c);
					state = STATE_STOP;
				}
			}
		}
		if (state == STATE_STOP) {
			state = STATE_IDLE;
			LL_I2C_DisableIT_EVT(i2c);
			LL_I2C_DisableIT_TX(i2c);
			LL_I2C_DisableIT_RX(i2c);
			TransCompleteCallback();
		}
	}
	
}

/*
 * Intterupt handler for I2C
 * (Using Bit POS option)
 * 
 *
void I2CHandleTypeDef::I2C_EV_IRQ_Handler() {
	if (state != STATE_IDLE) {
		if (LL_I2C_IsActiveFlag_AF(i2c) || LL_I2C_IsActiveFlag_ARLO(i2c) || LL_I2C_IsActiveFlag_BERR(i2c)) {
			LL_I2C_ClearFlag_AF(i2c);
			LL_I2C_ClearFlag_ARLO(i2c);
			LL_I2C_ClearFlag_BERR(i2c);
			LL_I2C_GenerateStopCondition(i2c);
			state = STATE_ERROR;
		}
		switch (state) {
		case STATE_START:
			if (LL_I2C_IsActiveFlag_SB(i2c)) {
				LL_I2C_TransmitData8(i2c, addr | 0);
				LL_I2C_EnableIT_TX(i2c);
				state = STATE_ADDR;
			}
			break;
		case STATE_REPSTART:
			if (LL_I2C_IsActiveFlag_SB(i2c)) {
				LL_I2C_TransmitData8(i2c, addr | 1);
				LL_I2C_EnableIT_RX(i2c);
				if (rxLen >= 2) {
					LL_I2C_AcknowledgeNextData(i2c, LL_I2C_ACK);
				}
				state = STATE_ADDR;
			}
			break;
		case STATE_ADDR:
			if (LL_I2C_IsActiveFlag_ADDR(i2c)) {
				if (txLen == 0 && rxLen > 0) {
					if (rxLen == 1) {
						LL_I2C_AcknowledgeNextData(i2c, LL_I2C_NACK);
						LL_I2C_ClearFlag_ADDR(i2c);
						LL_I2C_GenerateStopCondition(i2c);
						state = STATE_RX1;
					}
					else if (rxLen == 2) {
						LL_I2C_ClearFlag_ADDR(i2c);
						LL_I2C_AcknowledgeNextData(i2c, LL_I2C_NACK);
						state = STATE_RX2;
					}
					else if (rxLen > 2) {
						LL_I2C_ClearFlag_ADDR(i2c);
						LL_I2C_AcknowledgeNextData(i2c, LL_I2C_NACK);
						state = STATE_RX;
					}
				}
				else if (txLen > 0) {
					LL_I2C_ClearFlag_ADDR(i2c);
					state = STATE_TX;
				}
				else {
					LL_I2C_ClearFlag_ADDR(i2c);
					LL_I2C_GenerateStopCondition(i2c);
					state = STATE_STOP;
				}
			}
			break;
		case STATE_TX:
			if (LL_I2C_IsActiveFlag_TXE(i2c)) {
				if (txLen > 0) {
					txLen--;
					LL_I2C_TransmitData8(i2c, *txData);
					if (txLen > 0){txData++; }
				}
				else if (rxLen > 0) {
					LL_I2C_GenerateStartCondition(i2c);
					state = STATE_REPSTART;
				}
				else if (LL_I2C_IsActiveFlag_BTF(i2c)) {
					LL_I2C_GenerateStopCondition(i2c);
					state = STATE_STOP;
				}
			}
			break;
		case STATE_RX:
			if (rxLen > 3) {
				if (LL_I2C_IsActiveFlag_RXNE(i2c)) {
					rxLen--;
					*rxData = LL_I2C_ReceiveData8(i2c);
					rxData++;
				}
			}
			if (rxLen == 3) {
				if (LL_I2C_IsActiveFlag_RXNE(i2c)) {
					rxLen--;
				}
			}
			else if (rxLen == 2) {
				if (LL_I2C_IsActiveFlag_BTF(i2c)) {
					LL_I2C_AcknowledgeNextData(i2c, LL_I2C_NACK);
					*rxData = LL_I2C_ReceiveData8(i2c);
					rxData++;
					LL_I2C_GenerateStopCondition(i2c);
					rxLen--;
					*rxData = LL_I2C_ReceiveData8(i2c);
					rxData++;
				}
			}
			else {
				if (LL_I2C_IsActiveFlag_RXNE(i2c)) {
					rxLen--;
					*rxData = LL_I2C_ReceiveData8(i2c);
					state = STATE_STOP;
				}
			}
			break;
		case STATE_RX1:
			if (LL_I2C_IsActiveFlag_RXNE(i2c)) {
				rxLen--;
				*rxData = LL_I2C_ReceiveData8(i2c);
				state = STATE_STOP;
			}
			break;
		case STATE_RX2:
			if (LL_I2C_IsActiveFlag_BTF(i2c)) {
				rxLen--;
				*rxData = LL_I2C_ReceiveData8(i2c);
				rxData++;
				LL_I2C_GenerateStopCondition(i2c);
				rxLen--;
				*rxData = LL_I2C_ReceiveData8(i2c);
				state = STATE_STOP;
			}
			break;
		}
		if (state == STATE_STOP) {
			state = STATE_IDLE;
			LL_I2C_DisableIT_EVT(i2c);
			LL_I2C_DisableIT_TX(i2c);
			LL_I2C_DisableIT_RX(i2c);
			TransCompleteCallback();
		}
	}
	
}
*/

uint8_t I2CHandleTypeDef::WaitTransmission(uint16_t tout) {
	while (state != STATE_IDLE && state != STATE_ERROR && tout > 0) {
		tout -= LL_SYSTICK_IsActiveCounterFlag();
	}
	uint8_t t = state;
	state = STATE_IDLE;
	
	return t && tout > 0;
}