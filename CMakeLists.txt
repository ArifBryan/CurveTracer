cmake_minimum_required(VERSION 3.15)

project(CurveTracer LANGUAGES C CXX ASM)

find_bsp(
	ID com.sysprogs.arm.stm32
	VERSION 2022.01
	MCU STM32F103RE
	FRAMEWORKS com.sysprogs.arm.stm32.ll com.sysprogs.embedded.semihosting_and_profiler
	FWCONFIGURATION
		com.sysprogs.bspoptions.stm32.ll_driver=USE_FULL_LL_DRIVER
		com.sysprogs.efp.semihosting.buffer_size=4096
		com.sysprogs.efp.semihosting.stdio=1
		com.sysprogs.efp.semihosting.blocking_mode=1
		com.sysprogs.efp.profiling.semihosting_driver=1
		com.sysprogs.efp.profiling.counter=
		com.sysprogs.efp.profiling.address_validators=
		com.sysprogs.efp.profiling.nosampling=0
		com.sysprogs.efp.profiling.debugger_check=
		com.sysprogs.efp.profiling.rtos=USE_FREERTOS_IF_FOUND
		com.sysprogs.efp.profiling.hold_interrupts=
	HWREGISTER_LIST_FILE STM32F1xxxx/DeviceDefinitions/stm32f103xe.xml
	DISABLE_GNU_EXTENSIONS)

add_bsp_based_executable(
	NAME CurveTracer
	SOURCES
		main.cpp
		system_stm32f1xx.c
		stm32f1xx_hal_conf.h
		system.h
		system.cpp
		interrupt.c
		uart_it.h
		uart_it.cpp
		serial.cpp
		serial.h
		ILI9341.h
		ILI9341.cpp
		gfxfont.h
		LL_GFX.h
		glcdfont.c
		LL_GFX.cpp
		gfxfont.h
		LL_GFX.h
		INA226.cpp
		INA226.h
		I2CHandle.cpp
		I2CHandle.h
		userInterface.cpp
		userInterface.h
		outputControl.cpp
		outputControl.h
		XPT2046.cpp
		XPT2046.h
		ctLogoBitmap.h
		PID.h
		PID.cpp
		curveTracer.h
		curveTracer.cpp
		curveTracer.cpp
		curveTracer.h
	GENERATE_BIN
	GENERATE_MAP
	OUTPUT_RELOCATION_RECORDS)
set_source_files_properties(LL_SPITFT.h LL_SPITFT.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
