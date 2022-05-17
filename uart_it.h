#pragma once

#include <stm32f1xx_ll_usart.h>

#define SERIAL_RX_BUFF_SIZE	256
#define SERIAL_TX_BUFF_SIZE	SERIAL_RX_BUFF_SIZE

struct UART_IT_InitTypeDef {
	uint32_t TransferDirection = LL_USART_DIRECTION_TX_RX;
	uint32_t BaudRate;
	uint32_t DataWidth = LL_USART_DATAWIDTH_8B;
	uint32_t Parity = LL_USART_PARITY_NONE;
	uint32_t StopBits = LL_USART_STOPBITS_1;
};

struct UART_IT_TypeDef {
	UART_IT_TypeDef(USART_TypeDef *usart, volatile uint32_t *ticks) {
		this->usart = usart;
		this->ticks = ticks; }
	void Init(UART_IT_InitTypeDef *initStruct, void(*Receive_CallbackHandler)(char *data), char delimiter, uint32_t timeout);
	void Transmit(const char *data);
	void Handler();
	void IRQ_Handler();
private:
	volatile uint16_t txBuffPtr = 0;
	volatile uint16_t rxBuffPtr = 0;
	volatile char txBuff[SERIAL_TX_BUFF_SIZE + 1];
	volatile char rxBuff[SERIAL_RX_BUFF_SIZE + 1];
	char delimiter;
	uint32_t timeout;
	volatile uint32_t timeoutTmr;
	volatile uint32_t *ticks;
	USART_TypeDef *usart;
	void(*Receive_Handler)(char *data);
};