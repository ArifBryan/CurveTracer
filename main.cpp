#include <stm32f1xx_ll_bus.h>
#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_utils.h>

int main(void)
{
	LL_InitTick(72000000, 1000);

	//Warning: if the line below triggers an error, GPIOC is not connected to a AHB1 (Group 1) on this device.
	//In this case, please search the stm32xxxx_ll_bus.h file for 'PERIPH_GPIOC' to find out the correct
	//macro name and use it to replace LL_AHB1_GRP1_PERIPH_$$com.sysprogs.examples.lBedblink.LEDPORT$$ and LL_AHB1_GRP1_EnableClock() below. 
	LL_AHB1_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOC);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinOutputType(GPIOC, LL_GPIO_PIN_12, LL_GPIO_OUTPUT_PUSHPULL);
	LL_GPIO_SetPinSpeed(GPIOC, LL_GPIO_PIN_12, LL_GPIO_SPEED_FREQ_LOW);

	for (;;)
	{
		LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_12);
		LL_mDelay(500);
		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_12);
		LL_mDelay(500);
	}
}
