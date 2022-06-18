#pragma once

#include <stm32f1xx.h>
#include "outputControl.h"

#define  CT_MIN_SAMPLE_TIME	50
#define  CT_DATA_LEN	1000

struct SampleData_TypeDef {
	float v;
	float i;
};

struct CurveTracer_TypeDef {
	void Init(void);
	void Handler(void);
	void SetupChannel(Channel_TypeDef *ref, Channel_TypeDef *pointA) {
		SetupChannel(ref, pointA, 0);
	}
	void SetupChannel(Channel_TypeDef *ref, Channel_TypeDef *pointA, Channel_TypeDef *pointB) {
		pRef = ref;
		pA = pointA;
		pB = pointB;
	}
	void SetupParams2ch(float vStart, float vEnd, float vStep, float iLim, uint32_t tSample) {
		this->vStart = vStart;
		this->vEnd = vEnd;
		this->vStep = vStep;
		this->tSample = (tSample < CT_MIN_SAMPLE_TIME ? CT_MIN_SAMPLE_TIME : tSample);
		this->iLim = iLim;
	}
	void SetupParams3ch(float vStart, float vEnd, float vStep, float iLim, float iStart, float iEnd, float iStep, uint32_t tSample) {
		this->iStart = iStart;
		this->iEnd = iEnd;
		this->iStep = iStep;
		this->vStart = vStart;
		this->vEnd = vEnd;
		this->vStep = vStep;
		this->iLim = iLim;
		this->tSample = (tSample < CT_MIN_SAMPLE_TIME ? CT_MIN_SAMPLE_TIME : tSample);
	}
	uint32_t GetSampleLength(void) {return sampleLen;}
	uint32_t GetSampleCount(void) {return samplePtr;}
	void Start(void);
	void Stop(void);
	uint8_t IsSampling(void);
	uint8_t IsSamplingDone();
	uint8_t IsNewSample() {return newSample;}
	uint8_t IsNewSequence() {return nextSampleSeq;}
	uint32_t GetSequenceCount(void) {return sampleSeq;}
	uint8_t IsChannelValid(void) {return (pRef && pA) || (pRef && pA && pB);}
	SampleData_TypeDef data[CT_DATA_LEN];
	
	float vStart, vEnd, vStep, iLim;
	float iStart, iEnd, iStep;
	float vSample, iSample;
	float ibSample;
	uint32_t tSample;
private:
	uint8_t run;
	uint8_t end;
	Channel_TypeDef *pRef, *pA, *pB;
	uint32_t samplePtr;
	uint32_t sampleLen;
	uint32_t sampleSeq;
	uint8_t newSample;
	uint8_t nextSampleSeq;
};

extern CurveTracer_TypeDef curveTracer;