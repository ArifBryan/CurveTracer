#pragma once

#include "uart_it.h"

struct SCPI_TypeDef {
	SCPI_TypeDef(UART_IT_TypeDef *uart) {
		this->uart = uart;
	}
	void Init(void);
	void Handler(void);
	void Return(const char *data);
	void Return(float data);
	void UARTReceive_Handler(char *data);
private:
	bool strMatch(const char *str1, const char *str2);
	bool strSkim(char **str1, const char *str2);
	UART_IT_TypeDef *uart;
};

extern SCPI_TypeDef scpi;