#include "system.h"

System_TypeDef system;

#define UVLO_BYPASS 1

void(*Shutdown_Callback)(void);
void(*OverTemperature_Callback)(void);
extern "C" volatile uint32_t ticks;
uint32_t pwrBtnTmr;
uint32_t sledTmr;
uint32_t sysCheckTmr;
volatile uint16_t adcData[3];
volatile uint8_t adcDataIdx;
uint8_t overTemp;

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

extern "C" void ADC1_EOS_Handler() {
	adcData[adcDataIdx++] = LL_ADC_REG_ReadConversionData12(ADC1);
	if (adcDataIdx == 3){adcDataIdx = 0; }
	LL_ADC_REG_StartConversionSWStart(ADC1);
}

void System_TypeDef::Init(void(*Startup_CallbackHandler)(void), void(*Shutdown_CallbackHandler)(void), void(*OverTemperature_CallbackHandler)(void)) {
	RCC_Init();
	GPIO_Init();
	LL_GPIO_SetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
	ADC_Init();
	LL_mDelay(10);
	LL_GPIO_ResetOutputPin(LED_STA_GPIO, LED_STA_PIN);
	uint32_t platchTmr = Ticks();
	while (1) {
		// Power on button
		if (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN) && (ReadVsenseVin() >= 23000 || UVLO_BYPASS)) {
			if (Ticks() - pwrBtnTmr >= 200) {
				LL_GPIO_SetOutputPin(BEEPER_GPIO, BEEPER_PIN);
				LL_mDelay(50);
				LL_GPIO_ResetOutputPin(BEEPER_GPIO, BEEPER_PIN);
				LL_GPIO_SetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
				LL_GPIO_SetOutputPin(LED_PWR_GPIO, LED_PWR_PIN);
				// Peripheral init
				TIM_Init();
				UART_Init();
				SPI_Init();
				I2C_Init();
				EXTI_Init();
				// Startup handler
				Startup_CallbackHandler();
				Shutdown_Callback = Shutdown_CallbackHandler;
				OverTemperature_Callback = OverTemperature_CallbackHandler;
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
		}
		else if (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) {
			platchTmr = Ticks();
		}
	}
}

void System_TypeDef::Handler() {
	// ADC Vsense, Tsense sampling
	if (Ticks() - sysCheckTmr >= 100) {
		// Fan control
		if (ReadDriverTemp() > 33) {
			SetFanSpeed((ReadDriverTemp() - 33) * 4);
		}
		
		// Thermal supervisor
		if (ReadDriverTemp() > 70 && !overTemp) {
			overTemp = 1;
			OverTemperature_Callback();
		}
		else if (ReadDriverTemp() <= 65) {
			overTemp = 0;
		}
		
		// Power supervisor
		if (ReadVsenseVin() < 23000 && !UVLO_BYPASS) {
			Shutdown();
		}
		
		sysCheckTmr = Ticks();
	}
	
	// Status LED
	if (Ticks() - sledTmr >= 100 && !LL_GPIO_IsOutputPinSet(LED_STA_GPIO, LED_STA_PIN)) {
		LL_GPIO_SetOutputPin(LED_STA_GPIO, LED_STA_PIN);
		sledTmr = Ticks();
	}
	else if (Ticks() - sledTmr >= 500 && LL_GPIO_IsOutputPinSet(LED_STA_GPIO, LED_STA_PIN)) {
		LL_GPIO_ResetOutputPin(LED_STA_GPIO, LED_STA_PIN);
		sledTmr = Ticks();
	}
	
	// Power off button
	if (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) {
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
}

uint8_t System_TypeDef::OverTemperature() {
	return overTemp;
}

void System_TypeDef::Shutdown() {
	Shutdown_Callback();
	LL_GPIO_ResetOutputPin(LED_PWR_GPIO, LED_PWR_PIN);
	while (!LL_GPIO_IsInputPinSet(BTN_PWR_GPIO, BTN_PWR_PIN)) ;
	LL_GPIO_ResetOutputPin(PWR_LATCH_GPIO, PWR_LATCH_PIN);
	NVIC_SystemReset();
}

void System_TypeDef::RCC_Init() {
	LL_UTILS_PLLInitTypeDef PLLInit_Struct;
	
	PLLInit_Struct.Prediv = LL_RCC_PREDIV_DIV_1;
	PLLInit_Struct.PLLMul = LL_RCC_PLL_MUL_9;
	
	LL_UTILS_ClkInitTypeDef ClkInit_Struct;
	
	ClkInit_Struct.AHBCLKDivider = LL_RCC_SYSCLK_DIV_1;
	ClkInit_Struct.APB1CLKDivider = LL_RCC_APB1_DIV_2;
	ClkInit_Struct.APB2CLKDivider = LL_RCC_APB2_DIV_1;
	
	LL_PLL_ConfigSystemClock_HSE(8000000U, LL_UTILS_HSEBYPASS_OFF, &PLLInit_Struct, &ClkInit_Struct);
	LL_RCC_HSE_EnableCSS();
	LL_RCC_HSI_Disable();
	
	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	LL_InitTick(SystemCoreClock, 1000);
	LL_SYSTICK_EnableIT();
}

void System_TypeDef::GPIO_Init() {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOD);
	
	LL_GPIO_InitTypeDef GPIOInit_Struct;
	
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
	GPIOInit_Struct.Pull = LL_GPIO_PULL_DOWN;
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
	GPIOInit_Struct.Pin = SPI1_MISO_PIN;
	LL_GPIO_Init(SPI1_MISO_GPIO, &GPIOInit_Struct);
	GPIOInit_Struct.Pin = SPI1_SCK_PIN;
	LL_GPIO_Init(SPI1_SCK_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = SPI2_MOSI_PIN;
	LL_GPIO_Init(SPI2_MOSI_GPIO, &GPIOInit_Struct);
	GPIOInit_Struct.Pin = SPI2_MISO_PIN;
	LL_GPIO_Init(SPI2_MISO_GPIO, &GPIOInit_Struct);
	GPIOInit_Struct.Pin = SPI2_SCK_PIN;
	LL_GPIO_Init(SPI2_SCK_GPIO, &GPIOInit_Struct);
	
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
	
	GPIOInit_Struct.Pin = AD5541_CH1_NSS_PIN;
	LL_GPIO_Init(AD5541_CH1_NSS_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = AD5541_CH2_NSS_PIN;
	LL_GPIO_Init(AD5541_CH2_NSS_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = AD5541_CH3_NSS_PIN;
	LL_GPIO_Init(AD5541_CH3_NSS_GPIO, &GPIOInit_Struct);
	
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
	
	GPIOInit_Struct.Pin = VSENSE_USB_PIN;
	LL_GPIO_Init(VSENSE_USB_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = XPT2046_IRQ_PIN;
	LL_GPIO_Init(XPT2046_IRQ_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = INA226_CH1_INT_PIN;
	LL_GPIO_Init(INA226_CH1_INT_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = INA226_CH2_INT_PIN;
	LL_GPIO_Init(INA226_CH2_INT_GPIO, &GPIOInit_Struct);
	
	GPIOInit_Struct.Pin = INA226_CH3_INT_PIN;
	LL_GPIO_Init(INA226_CH3_INT_GPIO, &GPIOInit_Struct);
	
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
		
	LL_ADC_InitTypeDef ADCInit_Struct;
	
	ADCInit_Struct.DataAlignment = LL_ADC_DATA_ALIGN_RIGHT;
	ADCInit_Struct.SequencersScanMode = LL_ADC_SEQ_SCAN_DISABLE;
	
	LL_ADC_Init(ADC1, &ADCInit_Struct);
	
	LL_ADC_REG_InitTypeDef ADCREGInit_Struct;
	
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
	
	LL_ADC_Enable(ADC1);
	LL_ADC_EnableIT_EOS(ADC1);
	NVIC_EnableIRQ(ADC1_2_IRQn);
	
	LL_ADC_StartCalibration(ADC1);
	while (LL_ADC_IsCalibrationOnGoing(ADC1)) ;
	
	adcDataIdx = 0;
	LL_ADC_REG_StartConversionSWStart(ADC1);
}

void System_TypeDef::ADC_StartConv() {
	if (adcDataIdx == 3) {
		adcDataIdx = 0;
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
	return 50560.9 / adcData[2];
}

void System_TypeDef::TIM_Init() {
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
	
	LL_TIM_InitTypeDef TIMInit_Struct;
	
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
}

void System_TypeDef::SetFanSpeed(uint32_t spd) {
	spd *= 2.5;
	spd = (spd < 30 ? 0 : (spd > 250 ? 250 : spd));
	LL_TIM_OC_SetCompareCH4(FAN_TIM, spd);
}

void System_TypeDef::UART_Init() {}
void System_TypeDef::SPI_Init() {}
void System_TypeDef::I2C_Init() {}
void System_TypeDef::EXTI_Init() {}

uint32_t System_TypeDef::Ticks() {
	return ticks;
}