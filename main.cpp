#include "system.h"
#include "userInterface.h"
#include "outputControl.h"
#include "curveTracer.h"

#include "SysprogsProfiler.h"

void Startup_Handler() {
	//InitializeInstrumentingProfiler();
	scpi.Init();
	ui.Init();
	ui.SplashScreen();
	outCtl.Init();
	curveTracer.Init();
	ui.DrawStartupInfo();
	outCtl.SelfTest();
	ui.DrawStartupInfo();
	LL_mDelay(500);
	if (outCtl.GetSelftestResult() == 2) {
		while (!sys.IsPowerBtnPressed()) {
			sys.WatchdogReload();
			LL_mDelay(100);
		}
		sys.Restart();
	}
	ui.SetScreenMenu(0);
}

void Shutdown_Handler() {
	outCtl.DisableAllOutputs();
	ui.DisableBacklight();
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
		scpi.Handler();
		ui.Handler();
		outCtl.Handler();
		curveTracer.Handler();
	}
}