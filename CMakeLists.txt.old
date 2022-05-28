cmake_minimum_required(VERSION 3.15)

project(CurveTracer LANGUAGES C CXX ASM)

find_bsp(
	ID com.sysprogs.arm.stm32
	VERSION 2022.01
	MCU STM32F103RE
	FRAMEWORKS com.sysprogs.arm.stm32.ll
	FWCONFIGURATION com.sysprogs.bspoptions.stm32.ll_driver=USE_FULL_LL_DRIVER
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
	GENERATE_BIN
	GENERATE_MAP)
set_source_files_properties(LL_SPITFT.h LL_SPITFT.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
