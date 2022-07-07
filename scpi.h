#pragma once

#include "uart_it.h"

struct SCPIReg_TypeDef {
	bool DispState;
	char DispData[26];
	uint16_t BeepTime = 100;
	uint8_t BeepCount = 1;
};

struct SCPI_TypeDef {
	SCPI_TypeDef(UART_IT_TypeDef *uart) {
		this->uart = uart;
	}
	void Init(void);
	void Handler(void);
	void Return(const char *data);
	void ReturnRaw(const char *data);
	void Return(float data);
	void Return(int data);
	void UARTReceive_Handler(char *data);
	SCPIReg_TypeDef reg;
private:
	bool strMatch(const char *str1, const char *str2);
	bool strSkim(char **str1, const char *str2);
	bool IsMnemonic(char **str, const char *m);
	UART_IT_TypeDef *uart;
};

extern SCPI_TypeDef scpi;