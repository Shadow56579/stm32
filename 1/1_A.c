#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
#define delay_default 1000000

enum Anim{
	ANIM1_1 = 0,ANIM1_2 = 1,ANIM1_3 = 2,ANIM1_4 = 3,ANIM1_5 = 4,ANIM1_6 = 5,ANIM1_7 = 6,ANIM1_8 = 7,ANIM2_1 = 8,ANIM2_2 = 9
};

void delay(uint32_t);

int main(void)
{

	int8_t counter = ANIM1_1;

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIODEN);

	GPIOB->MODER &= ~(GPIO_MODER_MODE14_Msk);
	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk |
			GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) | (1 << GPIO_MODER_MODE3_Pos)
			| (1 << GPIO_MODER_MODE4_Pos) | (1 << GPIO_MODER_MODE5_Pos) | (1 << GPIO_MODER_MODE6_Pos) | (1 << GPIO_MODER_MODE7_Pos);

	while(1)
	{

		GPIOD->BSRR = (GPIO_BSRR_BR0) | (GPIO_BSRR_BR1) | (GPIO_BSRR_BR2) | (GPIO_BSRR_BR3) | (GPIO_BSRR_BR4) | (GPIO_BSRR_BR5)
				| (GPIO_BSRR_BR6) | (GPIO_BSRR_BR7);

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



		if(counter == ANIM1_1)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS0);
			counter = ANIM1_2;
			delay(delay_default/5);
		}

		else if(counter == ANIM1_2)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS1);
			counter = ANIM1_3;
			delay(delay_default/5);
		}

		else if(counter == ANIM1_3)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS2);
			counter = ANIM1_4;
			delay(delay_default/5);
		}

		else if(counter == ANIM1_4)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS3);
			counter = ANIM1_5;
			delay(delay_default/5);
		}

		else if(counter == ANIM1_5)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS4);
			counter = ANIM1_6;
			delay(delay_default/5);
		}

		else if(counter == ANIM1_6)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS5);
			counter = ANIM1_7;
			delay(delay_default/5);
		}

		else if(counter == ANIM1_7)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS6);
			counter = ANIM1_8;
			delay(delay_default/5);
		}

		else if(counter == ANIM1_8)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS7);
			counter = ANIM1_1;
			delay(delay_default/5);
		}

		else if(counter == ANIM2_1)
		{
			GPIOD->BSRR = (GPIO_BSRR_BS0) | (GPIO_BSRR_BS1) | (GPIO_BSRR_BS2) | (GPIO_BSRR_BS3) | (GPIO_BSRR_BS4) | (GPIO_BSRR_BS5)
					| (GPIO_BSRR_BS6) | (GPIO_BSRR_BS7);
			delay(delay_default/5);
			counter = ANIM2_2;
		}

		else
		{
			GPIOD->BSRR = (GPIO_BSRR_BR0) | (GPIO_BSRR_BR1) | (GPIO_BSRR_BR2) | (GPIO_BSRR_BR3) | (GPIO_BSRR_BR4) | (GPIO_BSRR_BR5)
					| (GPIO_BSRR_BR6) | (GPIO_BSRR_BR7);
			delay(delay_default/5);
			counter = ANIM2_1;
		}
	}

}


void delay(uint32_t delay_size)
{
	for(int i=0;i<delay_size;i++);
}
