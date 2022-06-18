#include "outputControl.h"
#include "system.h"

OutputControl_TypeDef outCtl;

volatile uint8_t spiDACTransCount;
volatile uint16_t spiDACBuffer[3];
volatile uint8_t adcReadChIndex;

#define LOOP_INTERVAL	0.01
#define LOOP_INTERVALms LOOP_INTERVAL * 1000

#define CH_STABLE_CNT	4

// INA226 CH1 IRQ line
extern "C" void EXTI4_Handler() {
	
}

// INA226 CH2 IRQ line
extern "C" void EXTI3_Handler() {
	
}

// INA226 CH3 IRQ line
extern "C" void EXTI9_Handler() {
	
}

// I2C1 transmission complete callback
void I2C1_TransComplete_Handler() {
	ina226Ch1.I2C_TransComplete_Handler();
	ina226Ch2.I2C_TransComplete_Handler();
	ina226Ch3.I2C_TransComplete_Handler();
	switch (adcReadChIndex) {
	case 0:
		if (ina226Ch1.IsReadComplete()) {
			adcReadChIndex = 1;
			ina226Ch2.ReadData();
		}
		break;
	case 1:
		if (ina226Ch2.IsReadComplete()) {
			adcReadChIndex = 2;
			ina226Ch3.ReadData();
		}
		break;
	case 2:
		if (ina226Ch3.IsReadComplete()) {
			adcReadChIndex = 0; 
		}
		break;
	}
}
extern "C" void SPI1_IRQHandler() {
	if (LL_SPI_IsActiveFlag_TXE(SPI1) && !LL_SPI_IsActiveFlag_BSY(SPI1) && spiDACTransCount) {
		spiDACTransCount--;
		switch (spiDACTransCount) {
		case 2:
			LL_GPIO_SetOutputPin(AD5541_CH1_NSS_GPIO, AD5541_CH1_NSS_PIN);
			LL_GPIO_ResetOutputPin(AD5541_CH2_NSS_GPIO, AD5541_CH2_NSS_PIN);
			LL_SPI_TransmitData16(SPI1, spiDACBuffer[1]);
			break;
		case 1:
			LL_GPIO_SetOutputPin(AD5541_CH2_NSS_GPIO, AD5541_CH2_NSS_PIN);
			LL_GPIO_ResetOutputPin(AD5541_CH3_NSS_GPIO, AD5541_CH3_NSS_PIN);
			LL_SPI_TransmitData16(SPI1, spiDACBuffer[2]);
			break;
		case 0:
			LL_GPIO_SetOutputPin(AD5541_CH3_NSS_GPIO, AD5541_CH3_NSS_PIN);
			LL_SPI_DisableIT_TXE(SPI1);
			break;
		}
	}
}

void OutputControl_TypeDef::Init() {
	ina226Ch1.Init(INA226_CONFIG_VBUSCT_204us, INA226_CONFIG_VSHCT_332us, INA226_CONFIG_AVG_64);
	ina226Ch2.Init(INA226_CONFIG_VBUSCT_204us, INA226_CONFIG_VSHCT_332us, INA226_CONFIG_AVG_64);
	ina226Ch3.Init(INA226_CONFIG_VBUSCT_204us, INA226_CONFIG_VSHCT_332us, INA226_CONFIG_AVG_64);
	
	ina226Ch1.SetCurrentCal(0.0521);
	ina226Ch2.SetCurrentCal(0.0521);
	ina226Ch3.SetCurrentCal(0.0521);
	
	//ch1.pidV.SetConstants(1.5, 0.005, 0.2, LOOP_INTERVAL); // Trial & error
	//ch1.pidV.SetConstants(1.8, 4.629, 0.054, LOOP_INTERVAL); // Ziegler Nichols
	ch1.pidV.SetConstants(1.8, 30.0, 0.005, LOOP_INTERVAL); // Trial & Error
	ch1.pidI.SetConstants(0.75, 80.0, 0.002, LOOP_INTERVAL);
	ch1.pidV.SetOutputRange(0, 0xFFFF);
	ch1.pidI.SetOutputRange(0, 0xFFFF);
	
	ch2.pidV.SetConstants(1.8, 30.0, 0.005, LOOP_INTERVAL);
	ch2.pidI.SetConstants(0.75, 80.0, 0.002, LOOP_INTERVAL);
	ch2.pidV.SetOutputRange(0, 0xFFFF);
	ch2.pidI.SetOutputRange(0, 0xFFFF);
	
	ch3.pidV.SetConstants(1.8, 30.0, 0.005, LOOP_INTERVAL);
	ch3.pidI.SetConstants(0.75, 80.0, 0.002, LOOP_INTERVAL);
	ch3.pidV.SetOutputRange(0, 0xFFFF);
	ch3.pidI.SetOutputRange(0, 0xFFFF);
		
	ch1.SetVoltage(0);
	ch1.SetCurrent(1000);
	ch2.SetVoltage(0);
	ch2.SetCurrent(1000);
	ch3.SetVoltage(0);
	ch3.SetCurrent(1000);
	
	ch1.SetCalValue(521, 22939, 0, 61149);
	ch2.SetCalValue(557, 22840, 0, 60822);
	ch3.SetCalValue(529, 22893, 0, 63765);
	
//	ch1.SelfCalibrate();
//	ch2.SelfCalibrate();
//	ch3.SelfCalibrate();
	
	SelfTest();
}

void OutputControl_TypeDef::Handler() {
	//if (sys.Ticks() - ctrlTimer >= LOOP_INTERVALms || sys.IsStartup()) {
//		ina226Ch1.ReadData();
//		
//		ch1.Handler();
//		ch2.Handler();
//		ch3.Handler();
//		
//		SetDACValue(1, ch1.mv);
//		SetDACValue(2, ch2.mv);
//		SetDACValue(3, ch3.mv);
//		
//		WriteDACValues();
		
		//ctrlTimer = sys.Ticks();
	//}
}

void OutputControl_TypeDef::Ticks10ms_IRQ_Handler() {
	if (++ctrlTimer >= LOOP_INTERVALms / 10) {
		ina226Ch1.ReadData();
		
		ch1.Handler();
		ch2.Handler();
		ch3.Handler();
		
		SetDACValue(1, ch1.mv);
		SetDACValue(2, ch2.mv);
		SetDACValue(3, ch3.mv);
		
		WriteDACValues();
		
		ctrlTimer = 0;
	}
}

void OutputControl_TypeDef::SetDACValue(uint8_t ch, uint16_t val) {
	spiDACBuffer[ch - 1] = val;
}

void OutputControl_TypeDef::WriteDACValues() {
	if (spiDACTransCount == 0) {
		spiDACTransCount = 3;
		LL_GPIO_ResetOutputPin(AD5541_CH1_NSS_GPIO, AD5541_CH1_NSS_PIN);
		LL_SPI_TransmitData16(SPI1, spiDACBuffer[0]);
		LL_SPI_EnableIT_TXE(SPI1);
	}
}

void OutputControl_TypeDef::DisableAllOutputs() {
	LL_GPIO_ResetOutputPin(OPA548_CH1_ES_GPIO, OPA548_CH1_ES_PIN);	
	LL_GPIO_ResetOutputPin(OPA548_CH2_ES_GPIO, OPA548_CH2_ES_PIN);	
	LL_GPIO_ResetOutputPin(OPA548_CH3_ES_GPIO, OPA548_CH3_ES_PIN);	
}

void OutputControl_TypeDef::SelfTest() {
	uint32_t testTout = sys.Ticks();
	uint8_t testSequence = 0;
	selftestResult = 0;
	ch1.SetState(1);
	ch2.SetState(1);
	ch3.SetState(1);
	ch1.SetVoltage(0);
	ch2.SetVoltage(0);
	ch3.SetVoltage(0);
	while (sys.Ticks() - testTout < 5000) {
		if (ch1.IsStable() && ch2.IsStable() && ch3.IsStable()) {
			if (testSequence == 0) {
				ch1.SetVoltage(OUT_MAX_V);
				ch2.SetVoltage(OUT_MAX_V);
				ch3.SetVoltage(OUT_MAX_V);
				testSequence++;
			}
			else if (testSequence == 1) {
				selftestResult = 1;
				break;
			}
		}
		LL_IWDG_ReloadCounter(IWDG);
	}
	ch1.SetState(0);
	ch2.SetState(0);
	ch3.SetState(0);
	if (!selftestResult) {selftestResult = 2; }
}

// Channel
#define FILTER_Kf	5
void Channel_TypeDef::Handler() {
	vMeas = (ina226->GetVoltage() + (vMeas * FILTER_Kf)) / (FILTER_Kf + 1);
	iMeas = (ina226->GetCurrent() + (iMeas * FILTER_Kf)) / (FILTER_Kf + 1);
	
	if (GetState() && calState == 1) {
		if ((invert ? -iMeas : iMeas) > iSet) {
			mode = CH_MODE_CURRENT;
		}
		else if ((invert ? -(vMeas - calVMax) : vMeas) > vSet) {
			mode = CH_MODE_VOLTAGE;
		}
		
		if (invert) {
			pidV.Calculate(vSet, -(vMeas - calVMax));
			pidI.Calculate(iSet, -iMeas);
		}
		else {
			pidV.Calculate(vSet, vMeas);
			pidI.Calculate(iSet, iMeas);
		}
		
		if (mode == CH_MODE_VOLTAGE) {
			mv = pidV.GetOutput();
			if (abs(pidV.GetError()) >= 1.0) {
				stableCounter = CH_STABLE_CNT;
			}
			else {
				stableCounter = (stableCounter > 0 ? stableCounter - 1 : stableCounter);
			}
		}
		else {
			mv = pidI.GetOutput();
			pidV.Reset();
			if (abs(pidI.GetError()) >= 0.04) {
				stableCounter = CH_STABLE_CNT;
			}
			else {
				stableCounter = (stableCounter > 0 ? stableCounter - 1 : stableCounter);
			}
		}
		mv = (invert ? calDACMax - mv : mv);
	}
	else {
		pidV.Reset();
		pidI.Reset();
		mode = CH_MODE_FLOATING;
		stableCounter = CH_STABLE_CNT;
		if (calState) {
			mv = (invert ? calDACMax : 0);
		}
	}
}

void Channel_TypeDef::SelfCalibrate() {
	uint8_t calSequence = 0;
	calState = 0;
	mv = 0;
	calVMax = 0;
	calVMin = sys.ReadVsenseVin();
	SetState(1);
	while (!calState) {
		switch (calSequence) {
		case 0:
			mv += 0xFFFF / 800;
			LL_mDelay(LOOP_INTERVALms * 5);
			if (vMeas > calVMax) {
				calVMax = vMeas;
				calDACMax = mv;
			}
			if (mv >= 0xFFFF) {calSequence++; }
			break;
		case 1:
			if (mv > 0) {mv -= 0xFFFF / 800; }
			LL_mDelay(LOOP_INTERVALms * 5);
			if (vMeas < calVMin) {
				calVMin = vMeas;
				calDACMin = mv;
			}
			if (mv == 0) {calSequence++; }
			break;
		case 2:
			SetState(0);
			pidV.SetOutputRange(calDACMin, calDACMax);
			pidI.SetOutputRange(calDACMin, calDACMax);
			if (calVMin >= OUT_MIN_V || calVMax <= OUT_MAX_V) {calState = 2; }
			else{calState = 1; }
			break;
		}
		LL_IWDG_ReloadCounter(IWDG);
	}
}

void Channel_TypeDef::Invert(uint8_t en) {
	invert = en;
	stableCounter = CH_STABLE_CNT;
	pidV.Reset();
	pidI.Reset();
}

uint8_t Channel_TypeDef::IsStable() {
	return stableCounter == 0;
}

void Channel_TypeDef::SetVoltage(float vSet) {
	vSet = abs(vSet) + OUT_MIN_V;
	this->vSet = (vSet < OUT_MIN_V ? OUT_MIN_V : (vSet > OUT_MAX_V ? OUT_MAX_V : vSet));
	stableCounter = CH_STABLE_CNT;
	mode = CH_MODE_VOLTAGE;	
}
void Channel_TypeDef::SetCurrent(float iSet) {
	iSet = abs(iSet);
	this->iSet = (iSet < OUT_MIN_I ? OUT_MIN_I : (iSet > OUT_MAX_I ? OUT_MAX_I : iSet));
	stableCounter = CH_STABLE_CNT;
	mode = CH_MODE_CURRENT;
}

float Channel_TypeDef::GetVoltage() {
	return (invert ? vMeas - calVMax + OUT_MIN_V : vMeas - OUT_MIN_V);
}
float Channel_TypeDef::GetCurrent() {
	return iMeas;
}

void Channel_TypeDef::SetState(uint8_t en) {
	if (en) {
		LL_GPIO_SetOutputPin(gpio, pin);
		mode = CH_MODE_VOLTAGE;
		stableCounter = CH_STABLE_CNT;
	}
	else {
		LL_GPIO_ResetOutputPin(gpio, pin);
		stableCounter = 0;
	}
}
uint8_t Channel_TypeDef::GetState() {
	return LL_GPIO_IsOutputPinSet(gpio, pin);
}