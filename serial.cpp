#include "serial.h"
#include "system.h"

Serial_TypeDef serial;

void Serial1Receive_Handler(char *data) {
	if (strncmp(data, "*IDN?", 5) == 0) {
		uart1.Transmit("Convetech,CurveTracer,SCT-2001,v1.00a");
	}
}

void Serial_TypeDef::Init() {
	UART_IT_InitTypeDef UARTITInit_Struct;
	
	UARTITInit_Struct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	UARTITInit_Struct.BaudRate = 115200;
	UARTITInit_Struct.DataWidth = LL_USART_DATAWIDTH_8B;
	UARTITInit_Struct.Parity = LL_USART_PARITY_NONE;
	UARTITInit_Struct.StopBits = LL_USART_STOPBITS_1;
	
	uart1.Init(&UARTITInit_Struct , Serial1Receive_Handler, '\n', 1000);
}

void Serial_TypeDef::Handler() {
	uart1.Handler();
}