#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

int main(void)
{
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIODEN);

	GPIOB->MODER &= ~(GPIO_MODER_MODE12_Msk | GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk | GPIO_MODER_MODE15_Msk);
	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) |
			(1 << GPIO_MODER_MODE3_Pos);
	GPIOB->MODER |= (0 << GPIO_MODER_MODE12_Pos) | (0 << GPIO_MODER_MODE13_Pos) | (0 << GPIO_MODER_MODE14_Pos) | (0 << GPIO_MODER_MODE15_Pos);

	while(1)
	{
		if((GPIOB->IDR & GPIO_IDR_ID12) == 0)
		{
			GPIOD->ODR |= GPIO_ODR_OD0;
		}

		else if(GPIOB->IDR & GPIO_IDR_ID12)
		{
			GPIOD->ODR &= ~GPIO_ODR_OD0;
		}

		if((GPIOB->IDR & GPIO_IDR_ID13) == 0)
		{
			GPIOD->ODR |= GPIO_ODR_OD1;
		}

		else if((GPIOB->IDR & GPIO_IDR_ID13))
		{
			GPIOD->ODR &= ~GPIO_ODR_OD1;
		}

		if((GPIOB->IDR & GPIO_IDR_ID14) == 0)
		{
			GPIOD->ODR |= GPIO_ODR_OD2;
		}

		else if(GPIOB->IDR & GPIO_IDR_ID14)
		{
			GPIOD->ODR &= ~GPIO_ODR_OD2;
		}

		if((GPIOB->IDR & GPIO_IDR_ID15) == 0)
		{
			GPIOD->ODR |= GPIO_ODR_OD3;
		}

		else if(GPIOB->IDR & GPIO_IDR_ID15)
		{
			GPIOD->ODR &= ~GPIO_ODR_OD3;
		}

	}
}



