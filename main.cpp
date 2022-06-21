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
	ui.DrawStartupInfo();
	outCtl.Init();
	curveTracer.Init();
	ui.DrawStartupInfo();
	LL_mDelay(500);
	if (outCtl.GetSelftestResult() == 2) {
		while (1) {
			if (sys.IsPowerBtnPressed()) {
				sys.Restart();
			}
			LL_IWDG_ReloadCounter(IWDG);
		}
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