#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
#define delay_default 1000000
//led(1,2,3,4) - PB(3,4,5,6)
//button(1,2,3) - PC(13,14,15)

enum Anim{
	ANIM1_1 = 0,ANIM1_2 = 1,ANIM1_3 = 2,ANIM1_4 = 3,ANIM2_1 = 4,ANIM2_2 = 5
};

void delay(int);

int main(void)
{

	int counter = ANIM1_1;

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIOCEN);
	GPIOC->MODER &= ~(GPIO_MODER_MODE14_Msk);
	GPIOB->MODER &= ~(GPIO_MODER_MODE3_Msk | GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk);
	GPIOB->MODER |= (1 << GPIO_MODER_MODE3_Pos) | (1 << GPIO_MODER_MODE4_Pos) | (1 << GPIO_MODER_MODE5_Pos) | (1 << GPIO_MODER_MODE6_Pos);

	while(1)
	{

		GPIOB->ODR &= ~(GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6);

		if((GPIOC->IDR & GPIO_IDR_ID14) == 0)
		{
			if(counter <= ANIM1_4)
			{
				counter = ANIM2_1;
			}

			else
			{
				counter = ANIM1_1;
			}

			while((GPIOC->IDR & GPIO_IDR_ID14) == 0);
		}



		switch(counter)
		{
			case ANIM1_1:
			{
				GPIOB->ODR |= (GPIO_ODR_OD3);
				counter = ANIM1_2;
				delay(delay_default/5);
				break;
			}

			case ANIM1_2:
			{
				GPIOB->ODR |= (GPIO_ODR_OD4);
				counter = ANIM1_3;
				delay(delay_default/5);
				break;
			}

			case ANIM1_3:
			{
				GPIOB->ODR |= (GPIO_ODR_OD5);
				counter = ANIM1_4;
				delay(delay_default/5);
				break;
			}

			case ANIM1_4:
			{
				GPIOB->ODR |= (GPIO_ODR_OD6);
				counter = ANIM1_1;
				delay(delay_default/5);
				break;
			}

			case ANIM2_1:
			{
				GPIOB->ODR |= (GPIO_ODR_OD3) | (GPIO_ODR_OD4) | (GPIO_ODR_OD5) | (GPIO_ODR_OD6);
				delay(delay_default/5);
				counter = ANIM2_2;
				break;
			}

			default:
			{
				GPIOB->ODR &= ~(GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6);
				delay(delay_default/5);
				counter = ANIM2_1;
				break;
			}
		}


	}
}


void delay(int delay_size)
{
	for(int i=0;i<delay_size;i++);
}




