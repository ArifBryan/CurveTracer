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
	void SetupParamsBJT(float vceStart, float vceEnd, float vceStep, float icLim, float ibStart, float ibEnd, float ibStep, uint32_t tSample) {
		this->bStart = ibStart;
		this->bEnd = ibEnd;
		this->bStep = ibStep;
		this->vStart = vceStart;
		this->vEnd = vceEnd;
		this->vStep = vceStep;
		this->iLim = icLim;
		this->tSample = (tSample < CT_MIN_SAMPLE_TIME ? CT_MIN_SAMPLE_TIME : tSample);
	}
	void SetupParamsFET(float vdsStart, float vdsEnd, float vdsStep, float idLim, float vgsStart, float vgsEnd, float vgsStep, uint32_t tSample) {
		this->bStart = vgsStart;
		this->bEnd = vgsEnd;
		this->bStep = vgsStep;
		this->vStart = vdsStart;
		this->vEnd = vdsEnd;
		this->vStep = vdsStep;
		this->iLim = idLim;
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
	uint8_t IsChannelValid(void) {
		return ((pRef && pA) || (pRef && pA && pB)) && ((pRef != pA) && (pA != pB) && (pB!=pRef));
	}
	uint8_t IsParamValid(void) {
		uint8_t vParamValid = 0;
		uint8_t iParamValid = 0;
		uint8_t bParamValid = 0;
		if (outCtl.IsInverted()) {
			vParamValid = vStart <= 0 && vEnd < 0 && vStep != 0;
			iParamValid = iLim < 0;
			bParamValid = bStart <= 0 && bEnd <= 0 && bStep <= 0;
		}
		else {
			vParamValid = vStart >= 0 && vEnd > 0 && vStep != 0;
			iParamValid = iLim > 0;
			bParamValid = bStart >= 0 && bEnd >= 0 && bStep >= 0;
		}
		return vParamValid && iParamValid && bParamValid;
	}
	void InvertParams() {
		vStart = -vStart;
		vEnd = -vEnd;
		vStep = -vStep;
		iLim = -iLim;
		bStart = -bStart;
		bEnd = -bEnd;
		bStep = -bStep;
	}
	void ResetParams() {
		vStart = 0;
		vEnd = 0;
		vStep = 0;
		iLim = 0;
		bStart = 0;
		bEnd = 0;
		bStep = 0;
		pRef = 0;
		pA = 0;
		pB = 0;
	}
	void SetpBiasModeCurrent() {bType = bTypeCurrent;}
	void SetpBiasModeVoltage() {bType = bTypeVoltage; }
	SampleData_TypeDef data[CT_DATA_LEN];
	
	float vStart, vEnd, vStep, iLim;
	float bStart, bEnd, bStep;
	float vSample, iSample;
	float bSample;
	uint8_t bType;
	uint32_t tSample;
	const uint8_t bTypeCurrent = 0;
	const uint8_t bTypeVoltage = 1;
	Channel_TypeDef *pRef, *pA, *pB;
private:
	uint8_t run;
	uint8_t end;
	uint32_t samplePtr;
	uint32_t sampleLen;
	uint32_t sampleSeq;
	uint8_t newSample;
	uint8_t nextSampleSeq;
};

extern CurveTracer_TypeDef curveTracer;