#include "curveTracer.h"
#include "system.h"
#include "outputControl.h"
#include "userInterface.h"

CurveTracer_TypeDef curveTracer;

uint32_t tracerTimer;

void CurveTracer_TypeDef::Init() {
	
}

void CurveTracer_TypeDef::Handler() {
	if (sys.Ticks() - tracerTimer >= tSample && run) {
		if (pRef->IsStable() && pA->IsStable() && pB->IsStable()) {
			data[samplePtr].i = pA->GetCurrent();
			data[samplePtr].v = pA->GetVoltage() - pRef->GetVoltage();
			samplePtr++;
			if (samplePtr >= sampleLen || data[samplePtr - 1].i > iLim) {
				run = 0;
				pRef->SetVoltage(OUT_MIN_V);
				pA->SetVoltage(OUT_MIN_V);
				pB->SetVoltage(OUT_MIN_V);
				outCtl.DisableAllOutputs();
				end = 1;
			}
			else if (samplePtr < sampleLen) {
				vSample += vStep;
				pA->SetVoltage(pRef->GetSetVoltage() + vSample);
			}
			newSample = 1;
		}
		
		tracerTimer = sys.Ticks();
	}
}

void CurveTracer_TypeDef::Start() {
	run = 1;
	end = 0;
	samplePtr = 0;
	vSample = vStart;
	pRef->SetVoltage(OUT_MIN_V);
	pA->SetVoltage(pRef->GetSetVoltage() + vSample);
	pB->SetVoltage(pRef->GetSetVoltage()) ;
	pRef->SetState(1);
	pA->SetState(1);
	pB->SetState(1);
}

void CurveTracer_TypeDef::Stop() {
	run = 0;
	outCtl.DisableAllOutputs();
}

uint8_t CurveTracer_TypeDef::IsNewSample(uint8_t clear) {
	uint8_t t = newSample;
	if (t && clear) {
		newSample = 0;
	}
	
	return t;
}

uint8_t CurveTracer_TypeDef::IsSamplingDone() {
	return end;
}

uint8_t CurveTracer_TypeDef::IsSampling() {
	return run;
}