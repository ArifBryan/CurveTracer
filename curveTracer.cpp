#include "curveTracer.h"
#include "system.h"
#include "outputControl.h"
#include "userInterface.h"

CurveTracer_TypeDef curveTracer;

uint32_t tracerTimer;

void CurveTracer_TypeDef::Init() {
	
}

void CurveTracer_TypeDef::Handler() {
	newSample = 0;
	if (sys.Ticks() - tracerTimer >= tSample && run) {
		uint8_t stable = 0;
		if (pB) {
			stable = pRef->IsStable() && pA->IsStable() && pB->IsStable();
		}
		else {
			stable = pRef->IsStable() && pA->IsStable();
		}
		if (stable) {
			if ((samplePtr >= sampleLen || abs(data[samplePtr - 1].i) > abs(iLim)) && samplePtr > 0) {
				if (pB && abs(bSample) < abs(bEnd)) {
					bSample += bStep;
					if (bType == bTypeCurrent) {
						pB->SetCurrent(bSample);
					}
					else if (bType == bTypeVoltage) {
						pB->SetVoltage(bSample);
					}
					vSample = vStart;
					pA->SetVoltage(pRef->GetSetVoltage() + vSample);
					nextSampleSeq = 1;
					samplePtr = 0;
					sampleSeq++;
				}
				else {
					Stop();
					end = 1;
				}
			}
			else {
				nextSampleSeq = 0;
				data[samplePtr].i = pMeasCurrent->GetCurrent();
				data[samplePtr].v = pMeasVoltage->GetVoltage() - pRef->GetVoltage();
				samplePtr++;
			}
			if (samplePtr < sampleLen && run && !nextSampleSeq) {
				vSample += vStep;
				pA->SetVoltage(pRef->GetSetVoltage() + vSample);
			}
			newSample = 1;
		}
		
		tracerTimer = sys.Ticks();
	}
}

uint8_t CurveTracer_TypeDef::Start() {
	if (!IsChannelValid() || !IsParamValid() || !IsMeasChannelValid()) {return 0;}
	run = 1;
	end = 0;
	this->tSample = (tSample < CT_MIN_SAMPLE_TIME ? CT_MIN_SAMPLE_TIME : tSample);
	sampleLen = abs((vStart - vEnd) / vStep) + 1;
	samplePtr = 0;
	sampleSeq = 0;
	nextSampleSeq = 1;
	vSample = vStart;
	bSample = bStart;
	pRef->SetCurrent(1000);
	pRef->SetVoltage(0);
	pRef->SetState(1);
	pA->SetCurrent(1000);
	pA->SetVoltage(pRef->GetSetVoltage() + vSample);
	pA->SetState(1);
	if (pB) {
		if (bType == bTypeCurrent) {
			pB->SetCurrent(bSample);
			pB->SetVoltage(pRef->GetSetVoltage() + (vEnd * 3));
		}
		else if (bType == bTypeVoltage) {
			pB->SetCurrent(1000);
			pB->SetVoltage(bSample);
		}
		pB->SetState(1);
	}
	tracerTimer = sys.Ticks();
	
	return 1;
}

void CurveTracer_TypeDef::Stop() {
	run = 0;
	if (pRef) {pRef->SetVoltage(0); }
	if (pA) {pA->SetVoltage(0); }
	if (pB) {pB->SetVoltage(0); }
	outCtl.DisableAllOutputs();
}

uint8_t CurveTracer_TypeDef::IsSamplingDone() {
	return end;
}

uint8_t CurveTracer_TypeDef::IsSampling() {
	return run;
}