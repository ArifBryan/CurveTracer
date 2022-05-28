#include <stm32f1xx_ll_cortex.h>
#include "I2CHandle.h"

uint8_t I2CHandleTypeDef::Read(uint8_t devAddr, uint8_t regAddr[], uint8_t regAddrLen, uint8_t data[], uint16_t dataLen, uint16_t tout) {
	uint8_t status = 0;
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
			while(IsBusy() && tout > 0) {
				tout -= LL_SYSTICK_IsActiveCounterFlag();
			}
			data[0] = Receive();
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
			data[0] = Receive();
			//Stop
			GenerateStop();
			while (IsBusy() && tout > 0) {
				tout -= LL_SYSTICK_IsActiveCounterFlag();
			}
			data[1] = Receive();			
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
				data[i] = Receive();
			}
			while (!IsTransmmisionComplete() && tout > 0) {
				tout -= LL_SYSTICK_IsActiveCounterFlag();
			}
			data[i] = Receive();
			//Stop
			GenerateStop();
			while (IsBusy() && tout > 0) {
				tout -= LL_SYSTICK_IsActiveCounterFlag();
			}
			data[i + 1] = Receive();
		}
	}
	else {
		//Stop
		GenerateStop();
		while (IsBusy() && tout > 0) {
			tout -= LL_SYSTICK_IsActiveCounterFlag();
		}
	}
		
	return (status);
}

uint8_t I2CHandleTypeDef::Write(uint8_t devAddr, uint8_t data[], uint16_t dataLen, uint16_t tout) {
	uint8_t status = 0;
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
		for(uint16_t i = 0 ; i < dataLen ; i++) {
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
	
	return (status && (tout > 0));
}