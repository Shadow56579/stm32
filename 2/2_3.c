#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
//led(1,2,3,4) - PB(3,4,5,6)
//button(1,2,3) - PC(13,14,15)

typedef struct
{
	int8_t error_counter;
	uint32_t delay_counter;

}AnimProperties;

void delay(uint32_t);
void run_win_animation();
void run_fail_animation(AnimProperties*);
int8_t check_fail_animation(AnimProperties*);

enum Anim{
	SET = 0, RESET = 1
};

int8_t password_input[3];
int8_t button_counter;

int main(void)
{

	AnimProperties PropertiesObject;

	for(uint32_t i=0;i<3;i++)
	{
		password_input[i] = 0;
	}
	button_counter = 0;
	PropertiesObject.error_counter = 0;
	PropertiesObject.delay_counter = 0;

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIODEN);
	RCC->APB2ENR |= (RCC_APB2ENR_SYSCFGEN);

	GPIOB->MODER &= ~(GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk | GPIO_MODER_MODE15_Msk);
	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk
			| GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) |
			(1 << GPIO_MODER_MODE3_Pos) | (1 << GPIO_MODER_MODE4_Pos) | (1 << GPIO_MODER_MODE5_Pos) | (1 << GPIO_MODER_MODE6_Pos) | (1 << GPIO_MODER_MODE7_Pos);

	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PB | SYSCFG_EXTICR4_EXTI14_PB | SYSCFG_EXTICR4_EXTI15_PB;
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	EXTI->IMR1 |= EXTI_IMR1_IM13 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15;
	EXTI->FTSR1 |= EXTI_FTSR1_FT13 | EXTI_FTSR1_FT14 | EXTI_FTSR1_FT15;

	while(1)
	{

		if(button_counter == 3)
		{
			if((password_input[0] == 2) && (password_input[1] == 3) && (password_input[2] == 1))
			{
				run_win_animation(&PropertiesObject);
			}

			else
			{
				run_fail_animation(&PropertiesObject);
			}

		}


	}
}

void delay(uint32_t delay_size)
{
	for(uint32_t i=0;i<delay_size;i++);
}

void EXTI15_10_IRQHandler()
{

	if((EXTI->PR1 & EXTI_PR1_PIF13) && password_input[button_counter] == 0)
	{
		password_input[button_counter] = 1;
		button_counter++;
		GPIOD->ODR |= (GPIO_ODR_OD0);
		EXTI->PR1 = EXTI_PR1_PIF13;
	}

	else if(EXTI->PR1 & EXTI_PR1_PIF13)
	{
		EXTI->PR1 = EXTI_PR1_PIF13;
	}

	if((EXTI->PR1 & EXTI_PR1_PIF14) && password_input[button_counter] == 0)
	{
		password_input[button_counter] = 2;
		button_counter++;
		GPIOD->ODR |= (GPIO_ODR_OD1);
		EXTI->PR1 = EXTI_PR1_PIF14;
	}

	else if(EXTI->PR1 & EXTI_PR1_PIF14)
	{
		EXTI->PR1 = EXTI_PR1_PIF14;
	}

	if((EXTI->PR1 & EXTI_PR1_PIF15) && password_input[button_counter] == 0)
	{
		password_input[button_counter] = 3;
		button_counter++;
		GPIOD->ODR |= (GPIO_ODR_OD2);
		EXTI->PR1 = EXTI_PR1_PIF15;
	}

	else if(EXTI->PR1 & EXTI_PR1_PIF15)
	{
		EXTI->PR1 = EXTI_PR1_PIF15;
	}

	if(button_counter == 3)
	{
		EXTI->IMR1 &= ~(EXTI_IMR1_IM13 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15);
	}

}

void run_win_animation()
{
	while(1)
	{
		GPIOD->ODR |= (GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7);
		delay(500000);
		GPIOD->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7);
		delay(500000);

	}

}

void run_fail_animation(AnimProperties* PropertiesPtr)
{

	GPIOD->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3
			| GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD6 | GPIO_ODR_OD7);

	while(1)
	{
		if( (check_fail_animation(PropertiesPtr)) == RESET)
		{
			return;
		}

		GPIOD->ODR = (GPIO_ODR_OD0);
		delay(500000);
		GPIOD->ODR = (GPIO_ODR_OD1);
		delay(500000);
		GPIOD->ODR = (GPIO_ODR_OD2);
		delay(500000);
		GPIOD->ODR = (GPIO_ODR_OD3);
		delay(500000);
		GPIOD->ODR = (GPIO_ODR_OD4);
		delay(500000);
		GPIOD->ODR = (GPIO_ODR_OD5);
		delay(500000);
		GPIOD->ODR = (GPIO_ODR_OD6);
		delay(500000);
		GPIOD->ODR = (GPIO_ODR_OD7);
		delay(500000);
	}


}

int8_t check_fail_animation(AnimProperties* PropertiesPtr)
{
	if( PropertiesPtr->error_counter <= 2)
	{
		PropertiesPtr->delay_counter++;
		if( PropertiesPtr->delay_counter == 3)
		{
			GPIOD->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6 | GPIO_ODR_OD7);

			PropertiesPtr->delay_counter = 0;
			PropertiesPtr->error_counter++;
			button_counter = 0;

			for(uint32_t i=0;i<3;i++)
			{
				password_input[i] = 0;
			}

			EXTI->IMR1 |= EXTI_IMR1_IM13 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15;

			return RESET;
		}
	}

	return SET;
}
