#include "system.h"
#include "userInterface.h"
#include "outputControl.h"
#include "Fonts/FreeSans9pt7b.h"
#include <stddef.h>

#define UVLO_BYPASS		0
#define PWRCTL_BYPASS	0

void(*Shutdown_Callback)(void);
void(*OverTemperature_Callback)(void);
extern "C" volatile uint32_t ticks;
volatile uint16_t adcData[3];
volatile uint8_t adcDataIdx;
uint32_t sledTmr;



System_TypeDef sys;
UART_IT_TypeDef uart1(USART1, &ticks);
ILI9341_TypeDef lcd(SPI2, LCD_NSS_GPIO, LCD_NSS_PIN, LCD_DC_GPIO, LCD_DC_PIN, SPI2_DMA, SPI2_DMA_TX_CH);
XPT2046_TypeDef ts(SPI2, XPT2046_NSS_GPIO, XPT2046_NSS_PIN, 1);
I2CHandleTypeDef i2c1(I2C1);
I2CHandleTypeDef i2c2(I2C2);
INA226_TypeDef ina226Ch1(&i2c1, 0b1000000, 1);
INA226_TypeDef ina226Ch2(&i2c1, 0b1000001, 1);
INA226_TypeDef ina226Ch3(&i2c1, 0b1000010, 1);

// Interrupt & exception handler
extern "C" void CSSFault_Handler() {
	SystemCoreClockUpdate();
	LL_Init1msTick(SystemCoreClock);
	Shutdown_Callback();
	uint8_t cnt = 0;
	while (1) {
		LL_GPIO_TogglePin(LED_STA_GPIO, LED_STA_PIN);
		LL_mDelay(100);
		if (cnt ++ >= 15) {break;}
	}
	NVIC_SystemReset();
}
extern "C" void HardFault_Handler() {
	outCtl.DisableAllOutputs();
	LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
	lcd.fillRect(295, 0, 25, 25, ILI9341_RED);
	while (1) ;
}

extern "C" void Ticks10ms_Handler() {
	sys.Ticks10ms_IRQ_Handler();
	outCtl.Ticks10ms_IRQ_Handler();
	ui.Ticks10ms_IRQ_Handler();
}

extern "C" void ADC1_EOS_Handler() {
	adcData[adcDataIdx++] = LL_ADC_REG_ReadConversionData12(ADC1);
	if (adcDataIdx == 3){adcDataIdx = 0; }
	else {LL_ADC_REG_StartConversionSWStart(ADC1);}
}

extern "C" void USART1_IRQHandler() {
	uart1.IRQ_Handler();
}

extern "C" void SPI2_IRQHandler() {
	lcd.SPI_IRQ_Handler();
	ts.SPI_IRQ_Handler();
}

extern "C" void I2C1_EV_IRQHandler() {
	i2c1.I2C_EV_IRQ_Handler();
}

extern void I2C1_TransComplete_Handler();

void System_TypeDef::Init(void(*Startup_CallbackHandler)(void), void(*Shutdown_CallbackHandler)(void), void(*OverTemperature_CallbackHandler)(void)) {
	Shutdown_Callback = Shutdown_CallbackHandler;
	OverTemperature_Callback = OverTemperature_CallbackHandler;
	// Check reset source
	uint8_t iwdgReset = LL_RCC_IsActiveFlag_IWDGRST();
	LL_RCC_ClearResetFlags();
	// Peripheral init
	RCC_Init();
	GPIO_Init();
	IWDG_Init();
	LL_GPIO_SetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
	// Wait for supply to stabilize
	LL_mDelay(10);
	// Check if system restart
	uint8_t forceOn = LL_RTC_BKP_GetRegister(BKP, LL_RTC_BKP_DR1) || iwdgReset;
	ADC_Init();
	LL_GPIO_ResetOutputPin(LED_STA_GPIO, LED_STA_PIN);
	uint32_t platchTmr = Ticks();
	while (1) {
		// Power on button
		if (((!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN) || forceOn) && (ReadVsenseVin() >= 23000 || UVLO_BYPASS)) || PWRCTL_BYPASS) {
			if (Ticks() - pwrBtnTmr >= 200) {
				LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
				LL_mDelay(50);
				LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
				LL_GPIO_SetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
				LL_GPIO_SetOutputPin(LED_PWR_GPIO, LED_PWR_PIN);
				// Peripheral init
				USART_Init();
				SPI_Init();
				I2C_Init();
				TIM_Init();
				DMA_Init();
				EXTI_Init();
				// Clear restart flag
				LL_RTC_BKP_SetRegister(BKP, LL_RTC_BKP_DR1, 0);
				// Intterupt priority management.
				// !IMPORTANT! Always manage interrupts priority wisely !IMPORTANT!
				// !IMPORTANT! Do not leave NVIC/Interrupt priority unmanaged !IMPORTANT!
				NVIC_Init();
				// Watchdog reset
				LL_IWDG_ReloadCounter(IWDG);
				// Error-caused Reset Handler
				if (iwdgReset) {
					ui.Init();
					lcd.setTextColor(ILI9341_BLACK);
					lcd.setCursor(5, 17);
					lcd.print("System Error!");
					lcd.setCursor(5, 37);
					lcd.print("Press power button to reset.");
					while (!IsPowerBtnPressed()) {
						LL_IWDG_ReloadCounter(IWDG); 
						LL_mDelay(100);
					}
					Restart();
				}
				// Startup handler
				Startup_CallbackHandler();
				LL_mDelay(100);
				// Watchdog reset
				LL_IWDG_ReloadCounter(IWDG);
				while (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) ;
				pwrBtnTmr = Ticks();
				break;
			}
		}
		else {
			pwrBtnTmr = Ticks();
		}
		
		// Temporary power latch timeout
		if (Ticks() - platchTmr >= 500) {
			LL_GPIO_ResetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
			
			platchTmr = Ticks();		
		}
		else if (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) {
			platchTmr = Ticks();
		}
		
		// Standby loop
		
		
		// Watchdog reset
		LL_IWDG_ReloadCounter(IWDG);
	}
}

void System_TypeDef::Handler() {
	// Startup flag
	startup = (startup > 0 ? 2 : 1);
	
	// Status LED
	if (Ticks() - sledTmr >= 250) {
		LL_GPIO_SetOutputPin(LED_STA_GPIO, LED_STA_PIN);
		if (overTemp || underVoltage) {
			LL_GPIO_TogglePin(LED_PWR_GPIO, LED_PWR_PIN);
		}
		else {
			LL_GPIO_SetOutputPin(LED_PWR_GPIO, LED_PWR_PIN);
		}
		// Start ADC conversion
		ADCStartConversion();
		
		sledTmr = Ticks();
	}
	
	// Watchdog reset
	LL_IWDG_ReloadCounter(IWDG);
}

uint8_t System_TypeDef::IsUndervoltage() {
	return underVoltage;
}

uint8_t System_TypeDef::IsStartup() {
	return startup < 2;
}

uint8_t System_TypeDef::IsPowerBtnPressed() {
	return !LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN);
}

void System_TypeDef::Ticks10ms_IRQ_Handler() {
	// Power off button
	if (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN) && !IsStartup()) {
		if (Ticks() - pwrBtnTmr >= 200) {
			LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
			LL_mDelay(50);
			LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
			Shutdown();
		}
	}
	else {
		pwrBtnTmr = Ticks();
	}
	// ADC Vsense, Tsense sampling
	if (Ticks() - sysCheckTmr >= 100) {
		// Fan control
		if (outCtl.ch1.GetState() || outCtl.ch2.GetState() || outCtl.ch3.GetState()) {
			if (ReadDriverTemp() > 31) {
				SetFanSpeed((ReadDriverTemp() - 31) * 15 + 25);
			}
			else {
				SetFanSpeed(25);
			}
		}
		else if (ReadDriverTemp() > 31) {
			SetFanSpeed((ReadDriverTemp() - 31) * 15);
		}
		else {
			SetFanSpeed(0);
		}
		
		// Thermal supervisor
		if (ReadDriverTemp() >= 45 && !overTemp) {
			overTemp = 1;
			OverTemperature_Callback();
		}
		else if (ReadDriverTemp() <= 40) {
			overTemp = 0;
		}
		
		// Power supervisor
		if (ReadVsenseVin() < 23000 && !UVLO_BYPASS) {
			underVoltage = 1;
			if (!PWRCTL_BYPASS) {
				Shutdown();
			}
		}
		else {
			underVoltage = 0;
		}
		
		sysCheckTmr = Ticks();
	}
}

uint8_t System_TypeDef::IsOverTemperature() {
	return overTemp;
}

void System_TypeDef::Shutdown() {
	Shutdown_Callback();
	LL_GPIO_ResetOutputPin(LED_PWR_GPIO, LED_PWR_PIN);
	while (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) ;
	LL_GPIO_ResetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
	NVIC_SystemReset();
	while (1) ;
}

void  System_TypeDef::Restart() {
	Shutdown_Callback();
	// Set restart flag
	LL_RTC_BKP_SetRegister(BKP, LL_RTC_BKP_DR1, 1);
	NVIC_SystemReset();
	while (1) ;
}

void System_TypeDef::RCC_Init() {
	LL_UTILS_PLLInitTypeDef PLLInit_Struct;
	
	PLLInit_Struct.Prediv = LL_RCC_PREDIV_DIV_1;
	// Overclock to 8MHz x 15 = 120MHz 
	PLLInit_Struct.PLLMul = LL_RCC_PLL_MUL_15;
	
	LL_UTILS_ClkInitTypeDef ClkInit_Struct;
	
	ClkInit_Struct.AHBCLKDivider = LL_RCC_SYSCLK_DIV_1;
	ClkInit_Struct.APB1CLKDivider = LL_RCC_APB1_DIV_1;
	ClkInit_Struct.APB2CLKDivider = LL_RCC_APB2_DIV_1;
	
	LL_PLL_ConfigSystemClock_HSE(8000000U, LL_UTILS_HSEBYPASS_OFF, &PLLInit_Struct, &ClkInit_Struct);
	LL_RCC_HSE_EnableCSS();
	LL_RCC_HSI_Disable();
	
	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	LL_InitTick(SystemCoreClock, 1000);
	LL_SYSTICK_EnableIT();
	
	LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_ALL);
	LL_APB2_GRP1_ForceReset(LL_APB2_GRP1_PERIPH_ALL);
	LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_ALL);
	LL_APB2_GRP1_ReleaseReset(LL_APB2_GRP1_PERIPH_ALL);
	
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_BKP);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	LL_PWR_EnableBkUpAccess();
}

void System_TypeDef::GPIO_Init() {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	
	LL_GPIO_InitTypeDef GPIOInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_GPIO_StructInit(&GPIOInit_Struct);
	
	// Communication pins
	
	// Comm. : UART
	GPIOInit_Struct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIOInit_Struct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIOInit_Struct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	
	GPIOInit_Struct.Pin = UART1_TX_PIN;
	LL_GPIO_Init(UART1_TX_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = UART3_TX_PIN;
	LL_GPIO_Init(UART3_TX_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Mode = LL_GPIO_MODE_INPUT;
	GPIOInit_Struct.Pull = LL_GPIO_PULL_UP;
	GPIOInit_Struct.Pin = UART1_RX_PIN;
	LL_GPIO_Init(UART1_RX_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = UART3_RX_PIN;
	LL_GPIO_Init(UART3_RX_GPIO, &GPIOInit_Struct);
	
	// Comm. : SPI
	GPIOInit_Struct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIOInit_Struct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIOInit_Struct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	
	GPIOInit_Struct.Pin = SPI1_MOSI_PIN;
	LL_GPIO_Init(SPI1_MOSI_GPIO, &GPIOInit_Struct);
	GPIOInit_Struct.Pin = SPI1_SCK_PIN;
	LL_GPIO_Init(SPI1_SCK_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = SPI2_MOSI_PIN;
	LL_GPIO_Init(SPI2_MOSI_GPIO, &GPIOInit_Struct);
	GPIOInit_Struct.Pin = SPI2_SCK_PIN;
	LL_GPIO_Init(SPI2_SCK_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Mode = LL_GPIO_MODE_INPUT;
	GPIOInit_Struct.Pull = LL_GPIO_PULL_DOWN;
	GPIOInit_Struct.Pin = SPI1_MISO_PIN;
	LL_GPIO_Init(SPI1_MISO_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = SPI2_MISO_PIN;
	LL_GPIO_Init(SPI2_MISO_GPIO, &GPIOInit_Struct);
	
	// Comm. : I2C
	GPIOInit_Struct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIOInit_Struct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIOInit_Struct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	
	GPIOInit_Struct.Pin = I2C1_SCL_PIN;
	LL_GPIO_Init(I2C1_SCL_GPIO, &GPIOInit_Struct);
	GPIOInit_Struct.Pin = I2C1_SDA_PIN;
	LL_GPIO_Init(I2C1_SDA_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = I2C2_SCL_PIN;
	LL_GPIO_Init(I2C2_SCL_GPIO, &GPIOInit_Struct);
	GPIOInit_Struct.Pin = I2C2_SDA_PIN;
	LL_GPIO_Init(I2C2_SDA_GPIO, &GPIOInit_Struct);
	
	// PWM pins
	GPIOInit_Struct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIOInit_Struct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIOInit_Struct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
	
	GPIOInit_Struct.Pin = FAN_PIN;
	LL_GPIO_Init(FAN_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = LCD_BKLT_PIN;
	LL_GPIO_Init(LCD_BKLT_GPIO, &GPIOInit_Struct);
	
	// High frequency output pins
	GPIOInit_Struct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIOInit_Struct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIOInit_Struct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	
	LL_GPIO_SetOutputPin(LCD_NSS_GPIO, LCD_NSS_PIN);
	GPIOInit_Struct.Pin = LCD_NSS_PIN;
	LL_GPIO_Init(LCD_NSS_GPIO, &GPIOInit_Struct);
	LL_GPIO_SetOutputPin(LCD_DC_GPIO, LCD_DC_PIN);
	GPIOInit_Struct.Pin = LCD_DC_PIN;
	LL_GPIO_Init(LCD_DC_GPIO, &GPIOInit_Struct);
	
	LL_GPIO_SetOutputPin(XPT2046_NSS_GPIO, XPT2046_NSS_PIN);
	GPIOInit_Struct.Pin = XPT2046_NSS_PIN;
	LL_GPIO_Init(XPT2046_NSS_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = AD5541_CH1_NSS_PIN;
	LL_GPIO_Init(AD5541_CH1_NSS_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = AD5541_CH2_NSS_PIN;
	LL_GPIO_Init(AD5541_CH2_NSS_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = AD5541_CH3_NSS_PIN;
	LL_GPIO_Init(AD5541_CH3_NSS_GPIO, &GPIOInit_Struct);
	
	// Low frequency output pins
	GPIOInit_Struct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIOInit_Struct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIOInit_Struct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	
	GPIOInit_Struct.Pin = LED_PWR_PIN;
	LL_GPIO_Init(LED_PWR_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = PWR_LATCH_PIN;
	LL_GPIO_Init(PWR_LATCH_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = LED_STA_PIN;
	LL_GPIO_Init(LED_STA_GPIO, &GPIOInit_Struct);
	LL_GPIO_AF_Remap_SWJ_NOJTAG();
	
	GPIOInit_Struct.Pin = BEEPER_PIN;
	LL_GPIO_Init(BEEPER_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = OPA548_CH1_ES_PIN;
	LL_GPIO_Init(OPA548_CH1_ES_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = OPA548_CH2_ES_PIN;
	LL_GPIO_Init(OPA548_CH2_ES_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = OPA548_CH3_ES_PIN;
	LL_GPIO_Init(OPA548_CH3_ES_GPIO, &GPIOInit_Struct);
	
	// Digital input pins
	GPIOInit_Struct.Mode = LL_GPIO_MODE_INPUT;
	GPIOInit_Struct.Pull = LL_GPIO_PULL_UP;
	
	GPIOInit_Struct.Pin = BTN_PWR_PIN;
	LL_GPIO_Init(BTN_PWR_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = XPT2046_IRQ_PIN;
	LL_GPIO_Init(XPT2046_IRQ_GPIO, &GPIOInit_Struct);
	LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTC, LL_GPIO_AF_EXTI_LINE8);
	
	GPIOInit_Struct.Pin = INA226_CH1_INT_PIN;
	LL_GPIO_Init(INA226_CH1_INT_GPIO, &GPIOInit_Struct);
	LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTB, LL_GPIO_AF_EXTI_LINE4);
	
	GPIOInit_Struct.Pin = INA226_CH2_INT_PIN;
	LL_GPIO_Init(INA226_CH2_INT_GPIO, &GPIOInit_Struct);
	LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTB, LL_GPIO_AF_EXTI_LINE3);
	
	GPIOInit_Struct.Pin = INA226_CH3_INT_PIN;
	LL_GPIO_Init(INA226_CH3_INT_GPIO, &GPIOInit_Struct);
	LL_GPIO_AF_SetEXTISource(LL_GPIO_AF_EXTI_PORTB, LL_GPIO_AF_EXTI_LINE9);
	
	GPIOInit_Struct.Pin = VSENSE_USB_PIN;
	GPIOInit_Struct.Pull = LL_GPIO_PULL_DOWN;
	LL_GPIO_Init(VSENSE_USB_GPIO, &GPIOInit_Struct);
	
	// Analog input pins
	GPIOInit_Struct.Mode = LL_GPIO_MODE_ANALOG;
	
	GPIOInit_Struct.Pin = VSENSE_5V_PIN;
	LL_GPIO_Init(VSENSE_5V_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = VSENSE_VIN_PIN;
	LL_GPIO_Init(VSENSE_VIN_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = TSENSE_PIN;
	LL_GPIO_Init(TSENSE_GPIO, &GPIOInit_Struct);
}

void System_TypeDef::ADC_Init() {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC1);
	LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSRC_PCLK2_DIV_2);
		
	LL_ADC_InitTypeDef ADCInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_ADC_StructInit(&ADCInit_Struct);
	
	ADCInit_Struct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
	ADCInit_Struct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
	
	LL_ADC_Init(ADC1, &ADCInit_Struct);
	
	LL_ADC_REG_InitTypeDef ADCREGInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_ADC_REG_StructInit(&ADCREGInit_Struct);
	
	ADCREGInit_Struct.ContinuousMode = LL_ADC_REG_CONV_SINGLE;
	ADCREGInit_Struct.DMATransfer = LL_ADC_REG_DMA_TRANSFER_NONE;
	ADCREGInit_Struct.SequencerDiscont = LL_ADC_REG_SEQ_DISCONT_1RANK;
	ADCREGInit_Struct.TriggerSource = LL_ADC_REG_TRIG_SOFTWARE;
	ADCREGInit_Struct.SequencerLength = LL_ADC_REG_SEQ_SCAN_ENABLE_3RANKS;
	
	LL_ADC_REG_Init(ADC1, &ADCREGInit_Struct);
		
	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_1, VSENSE_VIN_ADC_CH);
	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_2, VSENSE_5V_ADC_CH);
	LL_ADC_REG_SetSequencerRanks(ADC1, LL_ADC_REG_RANK_3, TSENSE_ADC_CH);
	
	LL_ADC_SetChannelSamplingTime(ADC1, VSENSE_VIN_ADC_CH, LL_ADC_SAMPLINGTIME_71CYCLES_5);
	LL_ADC_SetChannelSamplingTime(ADC1, VSENSE_5V_ADC_CH, LL_ADC_SAMPLINGTIME_71CYCLES_5);
	LL_ADC_SetChannelSamplingTime(ADC1, TSENSE_ADC_CH, LL_ADC_SAMPLINGTIME_71CYCLES_5);
	
	NVIC_EnableIRQ(ADC1_2_IRQn);
	LL_ADC_EnableIT_EOS(ADC1);
	LL_ADC_Enable(ADC1);
	
	LL_ADC_StartCalibration(ADC1);
	while (LL_ADC_IsCalibrationOnGoing(ADC1)) ;
	
	adcDataIdx = 0;
	LL_ADC_REG_StartConversionSWStart(ADC1);
}

void System_TypeDef::ADCStartConversion() {
	if (adcDataIdx == 0) {
		LL_ADC_REG_StartConversionSWStart(ADC1);		
	}
}

uint32_t System_TypeDef::ReadVsense5V() {
	return adcData[1] * 1.4681;
}

uint32_t System_TypeDef::ReadVsenseVin() {
	return adcData[0] * 8.94621;
}

float System_TypeDef::ReadDriverTemp() {
	if (adcData[2] == 0) {return 0;}
	return 50560.9 / adcData[2];
}

void System_TypeDef::TIM_Init() {
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
	
	LL_TIM_InitTypeDef TIMInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_TIM_StructInit(&TIMInit_Struct);
	
	// PWM generation
	TIMInit_Struct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	TIMInit_Struct.CounterMode = LL_TIM_COUNTERDIRECTION_UP;
	TIMInit_Struct.RepetitionCounter = 0;
	
	TIMInit_Struct.Prescaler = 1000;
	TIMInit_Struct.Autoreload = 500;
	LL_TIM_Init(FAN_TIM, &TIMInit_Struct);
	
	TIMInit_Struct.Prescaler = 6;
	TIMInit_Struct.Autoreload = 100;
	LL_TIM_Init(LCD_BKLT_TIM, &TIMInit_Struct);
	
	LL_TIM_OC_InitTypeDef TIMOCInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_TIM_OC_StructInit(&TIMOCInit_Struct);
	
	TIMOCInit_Struct.OCMode = LL_TIM_OCMODE_PWM1;
	TIMOCInit_Struct.OCState = LL_TIM_OCSTATE_ENABLE;
	TIMOCInit_Struct.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
	TIMOCInit_Struct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
	TIMOCInit_Struct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIMOCInit_Struct.CompareValue = 0;
	
	LL_TIM_OC_Init(FAN_TIM, FAN_TIM_CH, &TIMOCInit_Struct);
	LL_TIM_OC_Init(LCD_BKLT_TIM, LCD_BKLT_TIM_CH, &TIMOCInit_Struct);
	
	LL_TIM_SetClockSource(FAN_TIM, LL_TIM_CLOCKSOURCE_INTERNAL);
	LL_TIM_SetClockSource(LCD_BKLT_TIM, LL_TIM_CLOCKSOURCE_INTERNAL);
	
	LL_TIM_EnableAllOutputs(FAN_TIM);
	LL_TIM_EnableAllOutputs(LCD_BKLT_TIM);
	
	LL_TIM_EnableCounter(FAN_TIM);
	LL_TIM_EnableCounter(LCD_BKLT_TIM);
	
	// Interrupt timer
	TIMInit_Struct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV4;
	TIMInit_Struct.CounterMode = LL_TIM_COUNTERDIRECTION_UP;
	TIMInit_Struct.RepetitionCounter = 0;
	
	TIMInit_Struct.Prescaler = 18000;
	TIMInit_Struct.Autoreload = 40;
	LL_TIM_Init(TIM3, &TIMInit_Struct);
	
	TIMOCInit_Struct.OCMode = LL_TIM_OCMODE_ACTIVE;
	TIMOCInit_Struct.OCState = LL_TIM_OCSTATE_DISABLE;
	TIMOCInit_Struct.OCNState = LL_TIM_OCSTATE_DISABLE;
	TIMOCInit_Struct.CompareValue = TIMInit_Struct.Autoreload;
	LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIMOCInit_Struct);
	
	LL_TIM_EnableIT_CC1(TIM3);
	
	NVIC_EnableIRQ(TIM3_IRQn);
	LL_TIM_EnableCounter(TIM3);
}

void System_TypeDef::SetFanSpeed(uint32_t spd) {
	spd *= 2.5;
	spd = (spd < 30 ? 0 : (spd > 250 ? 250 : spd));
	LL_TIM_OC_SetCompareCH4(FAN_TIM, spd);
}
uint8_t System_TypeDef::GetFanSpeed() {
	return LL_TIM_OC_GetCompareCH4(FAN_TIM) / 2.5;
}

uint8_t System_TypeDef::IsUSBConnected() {
	return LL_GPIO_IsInputPinSet(VSENSE_USB_GPIO, VSENSE_USB_PIN);
}

void System_TypeDef::USART_Init() {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
}
void System_TypeDef::SPI_Init() {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
	
	LL_SPI_InitTypeDef SPIInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_SPI_StructInit(&SPIInit_Struct);
	
	SPIInit_Struct.Mode = LL_SPI_MODE_MASTER;
	SPIInit_Struct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPIInit_Struct.NSS = LL_SPI_NSS_SOFT;
	SPIInit_Struct.ClockPolarity = LL_SPI_POLARITY_LOW;
	SPIInit_Struct.ClockPhase = LL_SPI_PHASE_1EDGE;
	SPIInit_Struct.BitOrder = LL_SPI_MSB_FIRST;
	SPIInit_Struct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	SPIInit_Struct.CRCPoly = 1U;
	SPIInit_Struct.DataWidth = LL_SPI_DATAWIDTH_16BIT;
	SPIInit_Struct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;
	
	LL_SPI_Init(SPI1, &SPIInit_Struct);
	
	SPIInit_Struct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	SPIInit_Struct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
	
	LL_SPI_Init(SPI2, &SPIInit_Struct);
	
	NVIC_EnableIRQ(SPI1_IRQn);
	NVIC_EnableIRQ(SPI2_IRQn);
	
	LL_SPI_Enable(SPI1);
	LL_SPI_Enable(SPI2);
}
void System_TypeDef::I2C_Init() {
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C2);
	
	LL_I2C_InitTypeDef I2CInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_I2C_StructInit(&I2CInit_Struct);
	
	I2CInit_Struct.ClockSpeed = 400000U;
	I2CInit_Struct.DutyCycle = LL_I2C_DUTYCYCLE_2;
	I2CInit_Struct.OwnAddress1 = 0x01;
	I2CInit_Struct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
	I2CInit_Struct.PeripheralMode = LL_I2C_MODE_I2C;
	I2CInit_Struct.TypeAcknowledge = LL_I2C_ACK;
		
	i2c1.Init(&I2CInit_Struct, I2C1_TransComplete_Handler);
	i2c2.Init(&I2CInit_Struct);
	
	NVIC_EnableIRQ(I2C1_EV_IRQn);
}

void System_TypeDef::EXTI_Init() {
	LL_EXTI_InitTypeDef EXTIInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_EXTI_StructInit(&EXTIInit_Struct);
	
	EXTIInit_Struct.Mode = LL_EXTI_MODE_IT;
	EXTIInit_Struct.LineCommand = ENABLE;
	EXTIInit_Struct.Trigger = LL_EXTI_TRIGGER_FALLING;
	
	EXTIInit_Struct.Line_0_31 = INA226_CH1_INT_EXTI;
	//LL_EXTI_Init(&EXTIInit_Struct);
	//NVIC_EnableIRQ(EXTI4_IRQn);
	EXTIInit_Struct.Line_0_31 = INA226_CH2_INT_EXTI;
	//LL_EXTI_Init(&EXTIInit_Struct);
	//NVIC_EnableIRQ(EXTI3_IRQn);
	EXTIInit_Struct.Line_0_31 = INA226_CH3_INT_EXTI;
	//LL_EXTI_Init(&EXTIInit_Struct);
	//NVIC_EnableIRQ(EXTI9_5_IRQn);
	
	EXTIInit_Struct.Line_0_31 = XPT2046_IRQ_EXTI;
	//LL_EXTI_Init(&EXTIInit_Struct);
}

void System_TypeDef::DMA_Init() {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
	
	LL_DMA_InitTypeDef DMAInit_Struct;
	
	// !IMPORTANT! always initialize init struct !IMPORTANT! //
	LL_DMA_StructInit(&DMAInit_Struct);
	
	// SPI2 TX DMA
	DMAInit_Struct.Mode = LL_DMA_MODE_NORMAL;
	DMAInit_Struct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	DMAInit_Struct.Priority = LL_DMA_PRIORITY_MEDIUM;
	DMAInit_Struct.MemoryOrM2MDstAddress = 0;
	DMAInit_Struct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
	DMAInit_Struct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
	DMAInit_Struct.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetRegAddr(SPI2);
	DMAInit_Struct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
	DMAInit_Struct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
	DMAInit_Struct.NbData = 0;
	
	LL_DMA_Init(SPI2_DMA, SPI2_DMA_TX_CH, &DMAInit_Struct);
	
	// I2C1 TX DMA
	DMAInit_Struct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
	DMAInit_Struct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
	DMAInit_Struct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	DMAInit_Struct.PeriphOrM2MSrcAddress = LL_I2C_DMA_GetRegAddr(I2C1);
	DMAInit_Struct.NbData = 0;
	
	LL_DMA_Init(I2C1_DMA, I2C1_DMA_TX_CH, &DMAInit_Struct);
	
	// I2C1 RX DMA
	DMAInit_Struct.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
	DMAInit_Struct.PeriphOrM2MSrcAddress = LL_I2C_DMA_GetRegAddr(I2C1);
	DMAInit_Struct.NbData = 0;
	
	LL_DMA_Init(I2C1_DMA, I2C1_DMA_RX_CH, &DMAInit_Struct);
	
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

void System_TypeDef::IWDG_Init() {
	LL_IWDG_EnableWriteAccess(IWDG);
	LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_16);
	LL_IWDG_SetReloadCounter(IWDG, 0xFFF);
	LL_IWDG_Enable(IWDG);
	LL_IWDG_DisableWriteAccess(IWDG);
	LL_DBGMCU_APB1_GRP1_FreezePeriph(LL_DBGMCU_APB1_GRP1_IWDG_STOP);
}

void System_TypeDef::WatchdogReload() {
	LL_IWDG_ReloadCounter(IWDG);
}

void System_TypeDef::NVIC_Init() {
	// Intterupt priority management.
	// !IMPORTANT! Always manage interrupts priority wisely !IMPORTANT!
	// !IMPORTANT! Do not leave NVIC/Interrupt priority unmanaged !IMPORTANT!
	// Prioritize important interrupt with higher rank
	// Prioritize USART interrupt to avoid data overrun!
	NVIC_SetPriority(USART1_IRQn, 1);
	NVIC_SetPriority(I2C1_EV_IRQn, 2);
	NVIC_SetPriority(SPI2_IRQn, 2);
	NVIC_SetPriority(ADC1_2_IRQn, 2);
	NVIC_SetPriority(TIM3_IRQn, 3);
}

uint32_t System_TypeDef::Ticks() {
	return ticks;
}