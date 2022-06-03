#pragma once

#include <stm32f1xx.h>
#include "outputControl.h"

#define  CT_MIN_SAMPLE_TIME	50

struct SampleData_TypeDef {
	float v;
	float i;
};

struct CurveTracer_TypeDef {
	void Init(void);
	void Handler(void);
	void SetupChannel(Channel_TypeDef *ref, Channel_TypeDef *pointA, Channel_TypeDef *pointB) {
		pRef = ref;
		pA = pointA;
		pB = pointB;
	}
	void SetupParams(float vStart, float vEnd, float vStep, uint32_t tSample) {
		this->vStart = vStart;
		this->vEnd = vEnd;
		this->vStep = vStep;
		this->tSample = (tSample < CT_MIN_SAMPLE_TIME ? CT_MIN_SAMPLE_TIME : tSample);
		sampleLen = abs(vStart - vEnd) / vStep;
	}
	uint32_t GetSampleLength(void) {return sampleLen;}
	uint32_t GetSampleCount(void) {return samplePtr;}
	void Start(void);
	void Stop(void);
	uint8_t IsSampling(void);
	uint8_t IsSamplingDone(uint8_t clear);
	uint8_t IsSamplingDone() {return IsSamplingDone(0);}
	uint8_t IsNewSample(uint8_t clear);
	uint8_t IsNewSample() {return IsNewSample(0);}
	SampleData_TypeDef data[1000];
private:
	uint8_t run;
	Channel_TypeDef *pRef, *pA, *pB;
	float vStart, vEnd, vStep;
	float vSample, iSample;
	uint32_t tSample;
	uint32_t samplePtr;
	uint32_t sampleLen;
	uint8_t newSample;
};

extern CurveTracer_TypeDef curveTracer;