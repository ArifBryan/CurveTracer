#include "system.h"
#include "userInterface.h"
#include "outputControl.h"
#include "curveTracer.h"

#include "SysprogsProfiler.h"

void Startup_Handler() {
	//InitializeInstrumentingProfiler();
	
	scpi.Init();
	ui.Init();
	outCtl.Init();
	curveTracer.Init();
}

void Shutdown_Handler() {
	ui.SetBrightness(0);
}

void OverTemperature_Handler() {
	outCtl.DisableAllOutputs();
	if (curveTracer.IsSampling()) {
		curveTracer.Stop();
	}
	ui.ForceRedraw();
}

int main() {
	sys.Init(Startup_Handler, Shutdown_Handler, OverTemperature_Handler);
	while (1) {		
		sys.Handler();
		//scpi.Handler();
		ui.Handler();
		outCtl.Handler();
		curveTracer.Handler();
	}
}