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
		systemConfig.h
		systemConfig.cpp
		interrupt.c
	GENERATE_BIN
	GENERATE_MAP)
