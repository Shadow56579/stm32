#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
#define delay_default 1000000
//led(1,2,3,4) - PB(3,4,5,6)
//button(1,2,3) - PC(13,14,15)

enum Anim{
	ANIM1_1 = 0,ANIM1_2 = 1,ANIM1_3 = 2,ANIM1_4 = 3,ANIM1_5 = 4,ANIM1_6 = 5,ANIM1_7 = 6,ANIM1_8 = 7,ANIM2_1 = 8,ANIM2_2 = 9
};

void delay(int);

int main(void)
{

	int counter = ANIM1_1;

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIODEN);
	
	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk |
			GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
	GPIOB->MODER &= ~(GPIO_MODER_MODE14_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) |
			(1 << GPIO_MODER_MODE3_Pos) | (1 << GPIO_MODER_MODE4_Pos) | (1 << GPIO_MODER_MODE5_Pos) |
			(1 << GPIO_MODER_MODE6_Pos) | (1 << GPIO_MODER_MODE7_Pos);
	while(1)
	{

		GPIOD->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3 | GPIO_ODR_OD4 |
				GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7);

		if((GPIOB->IDR & GPIO_IDR_ID14) == 0)
		{
			if(counter <= ANIM1_8)
			{
				counter = ANIM2_1;
			}

			else
			{
				counter = ANIM1_1;
			}

			while((GPIOB->IDR & GPIO_IDR_ID14) == 0);
		}



		switch(counter)
		{
			case ANIM1_1:
			{
				GPIOD->ODR |= (GPIO_ODR_OD0);
				counter = ANIM1_2;
				delay(delay_default/5);
				break;
			}

			case ANIM1_2:
			{
				GPIOD->ODR |= (GPIO_ODR_OD1);
				counter = ANIM1_3;
				delay(delay_default/5);
				break;
			}

			case ANIM1_3:
			{
				GPIOD->ODR |= (GPIO_ODR_OD2);
				counter = ANIM1_4;
				delay(delay_default/5);
				break;
			}

			case ANIM1_4:
			{
				GPIOD->ODR |= (GPIO_ODR_OD3);
				counter = ANIM1_5;
				delay(delay_default/5);
				break;
			}
			case ANIM1_5:
			{
				GPIOD->ODR |= (GPIO_ODR_OD4);
				counter = ANIM1_6;
				delay(delay_default/5);
				break;
			}
			case ANIM1_6:
			{
				GPIOD->ODR |= (GPIO_ODR_OD5);
				counter = ANIM1_7;
				delay(delay_default/5);
				break;
			}
			case ANIM1_7:
			{
				GPIOD->ODR |= (GPIO_ODR_OD6);
				counter = ANIM1_8;
				delay(delay_default/5);
				break;
			}
			case ANIM1_8:
			{
				GPIOD->ODR |= (GPIO_ODR_OD7);
				counter = ANIM1_1;
				delay(delay_default/5);
				break;
			}
			case ANIM2_1:
			{
				GPIOD->ODR |= (GPIO_ODR_OD0) | (GPIO_ODR_OD1) | (GPIO_ODR_OD2) | (GPIO_ODR_OD3)
						| (GPIO_ODR_OD4) | (GPIO_ODR_OD5) | (GPIO_ODR_OD6) | (GPIO_ODR_OD7);
				delay(delay_default/5);
				counter = ANIM2_2;
				break;
			}

			default:
			{
				GPIOB->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5
						| GPIO_ODR_OD6 | GPIO_ODR_OD7);
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


