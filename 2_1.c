#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
//led(1,2,3,4) - PB(3,4,5,6)
//button(1,2,3) - PC(13,14,15)

void delay(uint32_t);
void view(int8_t);

int8_t is_pressed = 0;

int main(void)
{
	int8_t number = 0;

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIOCEN);
	RCC->APB2ENR |= (RCC_APB2ENR_SYSCFGEN);
	GPIOB->MODER &= ~(GPIO_MODER_MODE3_Msk | GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk);
	GPIOC->MODER &= ~(GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk | GPIO_MODER_MODE15_Msk);
	GPIOB->MODER |= (1 << GPIO_MODER_MODE3_Pos) | (1 << GPIO_MODER_MODE4_Pos) | (1 << GPIO_MODER_MODE5_Pos) | (1 << GPIO_MODER_MODE6_Pos);
	GPIOC->MODER |= (0 << GPIO_MODER_MODE13_Pos) | (0 << GPIO_MODER_MODE14_Pos) | (0 << GPIO_MODER_MODE15_Pos);
	SYSCFG->EXTICR[3] = SYSCFG_EXTICR4_EXTI14_PC;
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	EXTI->IMR1 |= EXTI_IMR1_IM14;
	EXTI->FTSR1 |= EXTI_FTSR1_FT14;

	while(1)
	{


		if(is_pressed == 1)
		{
			uint32_t counter = 0;

			while((GPIOC->IDR & GPIO_IDR_ID14) == 0)
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
	GPIOB->ODR &= ~(GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6);
	GPIOB->ODR |= (num & 0x0F) << 3;
}
