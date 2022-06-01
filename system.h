#pragma once

#include "stm32f1xx.h"
#include <stm32f1xx_ll_adc.h>
#include <stm32f1xx_ll_rcc.h>
#include <stm32f1xx_ll_bus.h>
#include <stm32f1xx_ll_exti.h>
#include <stm32f1xx_ll_tim.h>
#include <stm32f1xx_ll_cortex.h>
#include <stm32f1xx_ll_utils.h>
#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_usart.h>
#include <stm32f1xx_ll_i2c.h>
#include <stm32f1xx_ll_spi.h>
#include <stm32f1xx_ll_exti.h>
#include <stm32f1xx_ll_dma.h>
#include <stm32f1xx_ll_iwdg.h>
#include <stm32f1xx_ll_system.h>
#include <string.h>
#include <stdio.h>
#include "I2CHandle.h"
#include "uart_it.h"
#include "ILI9341.h"
#include "XPT2046.h"
#include "INA226.h"
#include <math.h>

// Firmware version
#define FW_VER_MAJOR	1
#define FW_VER_MINOR	20
#define FW_VER_REV		'a'
#define FW_VER_DATE		__DATE__

// GPIO pins
#define VSENSE_5V_GPIO		GPIOA
#define VSENSE_5V_PIN		LL_GPIO_PIN_1
#define VSENSE_5V_ADC_CH	LL_ADC_CHANNEL_1

#define VSENSE_VIN_GPIO		GPIOA
#define VSENSE_VIN_PIN		LL_GPIO_PIN_2
#define VSENSE_VIN_ADC_CH	LL_ADC_CHANNEL_2

#define TSENSE_GPIO			GPIOC
#define TSENSE_PIN			LL_GPIO_PIN_2
#define TSENSE_ADC_CH		LL_ADC_CHANNEL_12

#define VSENSE_USB_GPIO		GPIOA
#define VSENSE_USB_PIN		LL_GPIO_PIN_11

#define UART1_TX_GPIO		GPIOA
#define UART1_TX_PIN		LL_GPIO_PIN_9
#define UART1_RX_GPIO		GPIOA
#define UART1_RX_PIN		LL_GPIO_PIN_10

#define UART3_TX_GPIO		GPIOC
#define UART3_TX_PIN		LL_GPIO_PIN_10
#define UART3_RX_GPIO		GPIOC
#define UART3_RX_PIN		LL_GPIO_PIN_11

#define I2C1_SCL_GPIO		GPIOB
#define I2C1_SCL_PIN		LL_GPIO_PIN_6
#define I2C1_SDA_GPIO		GPIOB
#define I2C1_SDA_PIN		LL_GPIO_PIN_7
#define I2C1_DMA			DMA1
#define I2C1_DMA_TX_CH		LL_DMA_CHANNEL_6
#define I2C1_DMA_RX_CH		LL_DMA_CHANNEL_7

#define I2C2_SCL_GPIO		GPIOB
#define I2C2_SCL_PIN		LL_GPIO_PIN_10
#define I2C2_SDA_GPIO		GPIOB
#define I2C2_SDA_PIN		LL_GPIO_PIN_11

#define SPI1_SCK_GPIO		GPIOA
#define SPI1_SCK_PIN		LL_GPIO_PIN_5
#define SPI1_MISO_GPIO		GPIOA
#define SPI1_MISO_PIN		LL_GPIO_PIN_6
#define SPI1_MOSI_GPIO		GPIOA
#define SPI1_MOSI_PIN		LL_GPIO_PIN_7

#define SPI2_SCK_GPIO		GPIOB
#define SPI2_SCK_PIN		LL_GPIO_PIN_13
#define SPI2_MISO_GPIO		GPIOB
#define SPI2_MISO_PIN		LL_GPIO_PIN_14
#define SPI2_MOSI_GPIO		GPIOB
#define SPI2_MOSI_PIN		LL_GPIO_PIN_15
#define SPI2_DMA			DMA1
#define SPI2_DMA_TX_CH		LL_DMA_CHANNEL_5

#define INA226_CH1_INT_GPIO	GPIOB
#define INA226_CH1_INT_PIN	LL_GPIO_PIN_4
#define INA226_CH1_INT_EXTI	LL_EXTI_LINE_4
#define INA226_CH2_INT_GPIO	GPIOB
#define INA226_CH2_INT_PIN	LL_GPIO_PIN_3
#define INA226_CH2_INT_EXTI	LL_EXTI_LINE_3
#define INA226_CH3_INT_GPIO	GPIOB
#define INA226_CH3_INT_PIN	LL_GPIO_PIN_9
#define INA226_CH3_INT_EXTI	LL_EXTI_LINE_9

#define AD5541_CH1_NSS_GPIO	GPIOB
#define AD5541_CH1_NSS_PIN	LL_GPIO_PIN_5
#define AD5541_CH2_NSS_GPIO	GPIOD
#define AD5541_CH2_NSS_PIN	LL_GPIO_PIN_2
#define AD5541_CH3_NSS_GPIO	GPIOA
#define AD5541_CH3_NSS_PIN	LL_GPIO_PIN_4

#define OPA548_CH1_ES_GPIO	GPIOC
#define OPA548_CH1_ES_PIN	LL_GPIO_PIN_12
#define OPA548_CH2_ES_GPIO	GPIOC
#define OPA548_CH2_ES_PIN	LL_GPIO_PIN_0
#define OPA548_CH3_ES_GPIO	GPIOC
#define OPA548_CH3_ES_PIN	LL_GPIO_PIN_1

#define LED_PWR_GPIO		GPIOB
#define LED_PWR_PIN			LL_GPIO_PIN_0
#define PWR_LATCH_GPIO		GPIOB
#define PWR_LATCH_PIN		LL_GPIO_PIN_1
#define BTN_PWR_GPIO		GPIOB
#define BTN_PWR_PIN			LL_GPIO_PIN_2

#define LED_STA_GPIO		GPIOA
#define LED_STA_PIN			LL_GPIO_PIN_15

#define FAN_GPIO			GPIOA
#define FAN_PIN				LL_GPIO_PIN_3
#define FAN_TIM				TIM2
#define FAN_TIM_CH			LL_TIM_CHANNEL_CH4

#define XPT2046_NSS_GPIO	GPIOA
#define XPT2046_NSS_PIN		LL_GPIO_PIN_8

#define XPT2046_IRQ_GPIO	GPIOC
#define XPT2046_IRQ_PIN		LL_GPIO_PIN_8
#define XPT2046_IRQ_EXTI	LL_EXTI_LINE_8

#define LCD_NSS_GPIO		GPIOB
#define LCD_NSS_PIN			LL_GPIO_PIN_12

#define LCD_DC_GPIO			GPIOC
#define LCD_DC_PIN			LL_GPIO_PIN_7

#define LCD_BKLT_GPIO		GPIOB
#define LCD_BKLT_PIN		LL_GPIO_PIN_8
#define LCD_BKLT_TIM		TIM4
#define LCD_BKLT_TIM_CH		LL_TIM_CHANNEL_CH3

#define BEEPER_GPIO			GPIOC
#define BEEPER_PIN			LL_GPIO_PIN_6

#define IO1_GPIO			GPIOC
#define IO1_PIN				LL_GPIO_PIN_9

#define SWD_IO_GPIO			GPIOA
#define SWD_IO_PIN			LL_GPIO_PIN_13
#define SWD_CLK_GPIO		GPIOA
#define SWD_CLK_PIN			LL_GPIO_PIN_14

struct System_TypeDef {
	void Init(void(*Startup_CallbackHandler)(void), void(*Shutdown_CallbackHandler)(void), void(*OverTemperature_CallbackHandler)(void));
	void Handler(void);
	void Shutdown(void);
	uint8_t OverTemperature(void);
	uint32_t ReadVsense5V(void);
	uint32_t ReadVsenseVin(void);
	float ReadDriverTemp(void);
	uint32_t Ticks(void);
	uint8_t IsStartup(void);
	void Ticks10ms_IRQ_Handler();
private:
	void RCC_Init(void);
	void GPIO_Init(void);
	void ADC_Init(void);
	void TIM_Init(void);
	void USART_Init(void);
	void SPI_Init(void);
	void I2C_Init(void);
	void EXTI_Init(void);
	void DMA_Init(void);
	void IWDG_Init(void);
	void SetFanSpeed(uint32_t spd);
};

extern System_TypeDef sys;
extern UART_IT_TypeDef uart1;
extern ILI9341_TypeDef lcd;
extern XPT2046_TypeDef ts;
extern I2CHandleTypeDef i2c1;
extern I2CHandleTypeDef i2c2;
extern INA226_TypeDef ina226Ch1;
extern INA226_TypeDef ina226Ch2;
extern INA226_TypeDef ina226Ch3;