
#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void delay(int8_t);
void view(int8_t);


int main(void)
{

	int8_t number = 0;

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIODEN);

	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
	GPIOB->MODER &= ~(GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk | GPIO_MODER_MODE15_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) | (1 << GPIO_MODER_MODE3_Pos);
	GPIOB->MODER |= (0 << GPIO_MODER_MODE13_Pos) | (0 << GPIO_MODER_MODE14_Pos) | (0 << GPIO_MODER_MODE15_Pos);


	while(1)
	{

		if((GPIOB->IDR & GPIO_IDR_ID13) == 0)
		{
			number++;
			if(number >= 16) number = 0;
			view(number);
			while((GPIOB->IDR & GPIO_IDR_ID13) == 0);
		}

		if((GPIOB->IDR & GPIO_IDR_ID14) == 0)
		{
			number--;
			if(number < 0) number = 0;
			view(number);
			while((GPIOB->IDR & GPIO_IDR_ID14) == 0);
		}

		if((GPIOB->IDR & GPIO_IDR_ID15) == 0)
		{
			number = 0;
			view(number);
			while((GPIOC->IDR & GPIO_IDR_ID15) == 0);
		}

	}


}


void delay(int8_t delay_size)
{
	for(int i=0;i<delay_size;i++);
}

void view(int8_t num)
{

	GPIOD->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3);
	GPIOD->ODR |= (num & 0b00001111) << 0;
}
