#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void delay(uint32_t);
void view(int8_t);

int8_t is_pressed = 0;

int main(void)
{
	int8_t number = 0;

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIODEN);
	RCC->APB2ENR |= (RCC_APB2ENR_SYSCFGEN);

	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
	GPIOB->MODER &= ~(GPIO_MODER_MODE14_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) | (1 << GPIO_MODER_MODE3_Pos);
	GPIOB->MODER |= (0 << GPIO_MODER_MODE14_Pos);

	SYSCFG->EXTICR[3] = SYSCFG_EXTICR4_EXTI14_PB;
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	EXTI->IMR1 |= EXTI_IMR1_IM14;
	EXTI->FTSR1 |= EXTI_FTSR1_FT14;

	while(1)
	{


		if(is_pressed == 1)
		{
			uint32_t counter = 0;

			while((GPIOB->IDR & GPIO_IDR_ID14) == 0)
			{
				counter++;
				if(counter == 1000000)
				{
					break;
				}
			}

			if(counter < 1000000)
			{
				number++;
				if(number >= 16)
				{
					number = 0;
				}
			}

			else
			{
				number = 0;
			}

			is_pressed = 0;
			view(number);
		}


	}
}

void delay(uint32_t delay_size)
{
	for(int i=0;i<delay_size;i++);
}

void EXTI15_10_IRQHandler()
{
	is_pressed = 1;
	EXTI->PR1 |= EXTI_PR1_PIF14;
}

void view(int8_t num)
{
	GPIOD->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3);
	GPIOD->ODR |= (num & 0x0F) << 0;
}
