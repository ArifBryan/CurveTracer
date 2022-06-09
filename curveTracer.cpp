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
		uint8_t stable = 0;
		if (pB) {
			stable = pRef->IsStable() && pA->IsStable() && pB->IsStable();
		}
		else {
			stable = pRef->IsStable() && pA->IsStable();
		}
		if (stable) {
			if (nextSampleSeq) {
				samplePtr = 0;
				nextSampleSeq = 0;
			}
			else {
				data[samplePtr].i = pA->GetCurrent();
				data[samplePtr].v = pA->GetVoltage() - pRef->GetVoltage();
				samplePtr++;
			}
			if (samplePtr >= sampleLen || data[samplePtr - 1].i > iLim) {
				if (pB && ibSample < iEnd) {
					ibSample += iStep;
					pB->SetCurrent(ibSample);
					vSample = vStart;
					pA->SetVoltage(pRef->GetSetVoltage() + vSample);
					nextSampleSeq = 1;
				}
				else {
					Stop();
					end = 1;
				}
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
	nextSampleSeq = 0;
	vSample = vStart;
	ibSample = iStart;
	pRef->SetVoltage(OUT_MIN_V);
	pRef->SetState(1);
	pA->SetVoltage(pRef->GetSetVoltage() + vSample);
	pA->SetState(1);
	if (pB) {
		//pB->SetVoltage(pRef->GetSetVoltage());
		pB->SetCurrent(ibSample);
		pB->SetVoltage(pRef->GetSetVoltage() + vEnd);
		pB->SetState(1);
	}
	tracerTimer = sys.Ticks();
}

void CurveTracer_TypeDef::Stop() {
	run = 0;
	pRef->SetVoltage(OUT_MIN_V);
	pA->SetVoltage(OUT_MIN_V);
	if (pB) {
		pB->SetVoltage(OUT_MIN_V);
	}
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