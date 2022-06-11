#include "uart_it.h"
#include <string.h>

void UART_IT_TypeDef::Init(UART_IT_InitTypeDef *initStruct, void(*Receive_CallbackHandler)(volatile char *data), char delimiter, uint32_t timeout) {
	this->Receive_Handler = Receive_CallbackHandler;
	this->delimiter = delimiter;
	this->timeout = timeout;
	
	LL_USART_InitTypeDef USARTInit_Struct;
	
	LL_USART_StructInit(&USARTInit_Struct);
	
	USARTInit_Struct.TransferDirection = initStruct->TransferDirection;
	USARTInit_Struct.BaudRate = initStruct->BaudRate;
	USARTInit_Struct.DataWidth = initStruct->DataWidth;
	USARTInit_Struct.Parity = initStruct->Parity;
	USARTInit_Struct.StopBits = initStruct->StopBits;
	USARTInit_Struct.OverSampling = LL_USART_OVERSAMPLING_16;
	USARTInit_Struct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	
	LL_USART_DisableSCLKOutput(usart);
	LL_USART_Init(usart, &USARTInit_Struct);
	
	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_EnableIRQ(USART3_IRQn);
	LL_USART_EnableIT_RXNE(usart);
	LL_USART_EnableIT_TC(usart);
	LL_USART_Enable(usart);
}

void UART_IT_TypeDef::IRQ_Handler() {
	if (LL_USART_IsActiveFlag_RXNE(usart)) {
		volatile char data = LL_USART_ReceiveData8(usart);
		
		timeoutTmr = *ticks;
		if (data == delimiter) {
			rxBuff[rxBuffPtr++] = data;
			rxBuff[rxBuffPtr] = 0;
			rxBuffPtr = 0;
		}
		else {
			rxBuff[rxBuffPtr++] = data;
			if (rxBuffPtr >= SERIAL_RX_BUFF_SIZE) {rxBuffPtr = SERIAL_RX_BUFF_SIZE;}
		}
		
		LL_USART_ClearFlag_RXNE(usart);
	}
	if (LL_USART_IsActiveFlag_TC(usart) && LL_USART_IsActiveFlag_TXE(usart)) {
		if (txBuff[txBuffPtr] != 0) {
			LL_USART_TransmitData8(usart, txBuff[txBuffPtr++]);
		}
		else {
			txBuffPtr = 0;
		}
		
		LL_USART_ClearFlag_TC(usart);
	}
}

void UART_IT_TypeDef::Handler() {
	if (rxBuff[0] != 0) {
		if (rxBuffPtr != 0 && *ticks - timeoutTmr >= timeout) {
			memset((char*)rxBuff, 0, strlen((char*)rxBuff));
			rxBuffPtr = 0;
		}
		else if (rxBuffPtr == 0) {
			Receive_Handler(rxBuff);
			memset((char*)rxBuff, 0, strlen((char*)rxBuff));
		}
	}
}

void UART_IT_TypeDef::Transmit(const char *data) {
	while (txBuffPtr != 0) ;
	memcpy((char*)txBuff, data, strlen(data));
	txBuff[strlen(data)] = delimiter;
	txBuff[strlen(data) + 1] = 0;
	LL_USART_TransmitData8(usart, txBuff[txBuffPtr++]);
}