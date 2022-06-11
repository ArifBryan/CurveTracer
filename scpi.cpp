#include "scpi.h"
#include "system.h"
#include "outputControl.h"
#include <string.h>

SCPI_TypeDef scpi(&uart1);

void SCPI_UARTReceive_Handler(volatile char *data) {
	scpi.UARTReceive_Handler((char*)data);
}

void SCPI_TypeDef::UARTReceive_Handler(char *data) {
	Return(data);
	// SCPI Command Header
	if (strSkim(&data, "*")) {
		if (strMatch(data, "IDN?")) {
			Return("Convetech,CurveTracer,SCT-2001,v1.00a");
		}
	}
	// SCPI Instrument Control Header
	else if(strSkim(&data, ":")) {
		// SOURce1
		if (strSkim(&data, "SOURce1:") || strSkim(&data, "SOUR1:")) {
			// VOLTage
			if (strSkim(&data, "VOLTage") || strSkim(&data, "VOLT")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch1.GetSetVoltage());
				}
				// Command
				else if (strSkim(&data, " ")) {
					outCtl.ch1.SetVoltage(atof(data));
				}
			}
			// CURRent
			else if (strSkim(&data, "CURRent") || strSkim(&data, "CURR")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch1.GetSetCurrent());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch1.SetCurrent(atof(data));
				}
			}
		}
		// SOURce2
		else if (strSkim(&data, "SOURce2:") || strSkim(&data, "SOUR2:")) {
			// VOLTage
			if (strSkim(&data, "VOLTage") || strSkim(&data, "VOLT")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch2.GetSetVoltage());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch2.SetVoltage(atof(data));
				}
			}
			// CURRent
			else if(strSkim(&data, "CURRent") || strSkim(&data, "CURR")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch2.GetSetCurrent());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch2.SetCurrent(atof(data));
				}
			}
		}
		// SOURce3
		else if(strSkim(&data, "SOURce3:") || strSkim(&data, "SOUR3:")) {
			// VOLTage
			if (strSkim(&data, "VOLTage") || strSkim(&data, "VOLT")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch3.GetSetVoltage());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch3.SetVoltage(atof(data));
				}
			}
			// CURRent
			else if(strSkim(&data, "CURRent") || strSkim(&data, "CURR")) {
				// Query
				if (strMatch(data, "?")) {
					Return(outCtl.ch3.GetSetCurrent());
				}
				// Command
				else if(strSkim(&data, " ")) {
					outCtl.ch3.SetCurrent(atof(data));
				}
			}
		}
	}
}

void SCPI_TypeDef::Init() {
	UART_IT_InitTypeDef UARTITInit_Struct;
	
	UARTITInit_Struct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	UARTITInit_Struct.BaudRate = 115200;
	UARTITInit_Struct.DataWidth = LL_USART_DATAWIDTH_8B;
	UARTITInit_Struct.Parity = LL_USART_PARITY_NONE;
	UARTITInit_Struct.StopBits = LL_USART_STOPBITS_1;
	
	uart->Init(&UARTITInit_Struct, SCPI_UARTReceive_Handler, '\n', 1000);
}

void SCPI_TypeDef::Handler() {
	uart->Handler();
}

void SCPI_TypeDef::Return(const char *data) {
	uart->Transmit(data);
}

void SCPI_TypeDef::Return(float data) {
	char str[13];
	sprintf(str, "%d.%1d", (int16_t)data, (uint16_t)(abs(data) * 10) % 10);
	uart->Transmit(str);
}

bool SCPI_TypeDef::strMatch(const char *str1, const char *str2) {
	bool s = strncmp(str1, str2, strlen(str2)) == 0;
	return s;
}
bool SCPI_TypeDef::strSkim(char **str1, const char *str2) {
	bool s = strncmp(*str1, str2, strlen(str2)) == 0;
	if (s) {
		*str1 += strlen(str2);
	}
	return s;
}