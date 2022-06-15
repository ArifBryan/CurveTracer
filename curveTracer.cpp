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
			if ((samplePtr >= sampleLen || data[samplePtr - 1].i > iLim) && samplePtr > 0) {
				if (pB && ibSample < iEnd) {
					ibSample += iStep;
					pB->SetCurrent(ibSample);
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
				data[samplePtr].i = pA->GetCurrent();
				data[samplePtr].v = pA->GetVoltage() - pRef->GetVoltage();
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

void CurveTracer_TypeDef::Start() {
	if (!IsChannelValid()) {return;}
	run = 1;
	end = 0;
	this->tSample = (tSample < CT_MIN_SAMPLE_TIME ? CT_MIN_SAMPLE_TIME : tSample);
	sampleLen = abs(vStart - vEnd) / vStep + 1;
	samplePtr = 0;
	sampleSeq = 0;
	nextSampleSeq = 1;
	vSample = vStart;
	ibSample = iStart;
	pRef->SetVoltage(OUT_MIN_V);
	pRef->SetState(1);
	pA->SetVoltage(pRef->GetSetVoltage() + vSample);
	pA->SetState(1);
	if (pB) {
		//pB->SetVoltage(pRef->GetSetVoltage());
		pB->SetCurrent(ibSample);
		pB->SetVoltage(pRef->GetSetVoltage() + (vEnd * 3));
		pB->SetState(1);
	}
	tracerTimer = sys.Ticks();
}

void CurveTracer_TypeDef::Stop() {
	run = 0;
	if (pRef) {pRef->SetVoltage(OUT_MIN_V); }
	if (pA) {pA->SetVoltage(OUT_MIN_V); }
	if (pB) {pB->SetVoltage(OUT_MIN_V); }
	outCtl.DisableAllOutputs();
}

uint8_t CurveTracer_TypeDef::IsSamplingDone() {
	return end;
}

uint8_t CurveTracer_TypeDef::IsSampling() {
	return run;
}