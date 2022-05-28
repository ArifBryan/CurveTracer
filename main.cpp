#include "system.h"
#include "serial.h"
#include "userInterface.h"
#include "outputControl.h"

#include "SysprogsProfiler.h"

void Startup_Handler() {
	//InitializeInstrumentingProfiler();
	
	serial.Init();
	ui.Init();
	outCtl.Init();
}

void Shutdown_Handler() {
	ui.SetBrightness(0);
}

void OverTemperature_Handler() {
	outCtl.SetOutputState(0);
}

int main() {
	system.Init(Startup_Handler, Shutdown_Handler, OverTemperature_Handler);
	while (1) {		
		system.Handler();
		serial.Handler();
		ui.Handler();
		outCtl.Handler();		
	}
}