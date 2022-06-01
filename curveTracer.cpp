#include "curveTracer.h"
#include "system.h"
#include "outputControl.h"

CurveTracer_TypeDef curveTracer;

uint32_t tracerTimer;
uint16_t tracerCounter;
uint8_t nextStep;
float voltageStep;
float traceVoltage;
uint16_t lcdOffsX;
uint16_t lcdOffsY;

uint16_t lastPoint[2];

void CurveTracer_TypeDef::Init() {
	voltageStep = 2;
}

void CurveTracer_TypeDef::Handler() {
	if (sys.Ticks() - tracerTimer >= 100 && run) {
		if (nextStep) {
			nextStep = 0;
			traceVoltage += voltageStep;
			outCtl.ch2.SetVoltage(traceVoltage);
		}
		else if (outCtl.ch2.IsStable()) {
			uint16_t y = (uint16_t)outCtl.ch2.GetCurrent();
			if (y < 182) {
				y = (y > 182 ? 182 : (y < 0 ? 0 : y));
				uint16_t point[2];
				point[0] = 20 + tracerCounter;
				point[1] = 220 - y;
				lcd.drawLine(lastPoint[0], lastPoint[1], point[0], point[1], ILI9341_ORANGE);
				lastPoint[0] = point[0];
				lastPoint[1] = point[1];
			}
			else {
				lastPoint[0] = 20 + tracerCounter;
				lastPoint[1] = 220;
			}
			tracerCounter++;
			nextStep = 1;
			if (tracerCounter >= 190) {
				run = 0;
				outCtl.ch2.SetVoltage(1500);
			}
		}
		
		tracerTimer = sys.Ticks();
	}
}

void CurveTracer_TypeDef::Start() {
	run = 1;
	nextStep = 1;
	tracerTimer = 100;
	tracerCounter = 0;
	traceVoltage = 2100;
	lastPoint[0] = 20;
	lastPoint[1] = 220;
}