#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
//led(1,2,3,4) - PB(3,4,5,6)
//button(1,2,3) - PC(13,14,15)

typedef struct
{
	int8_t key[10];
	int8_t error_counter;
	uint32_t delay_counter;
	int8_t password_input[10];
	int8_t button_counter;

}AnimProperties;

void delay(uint32_t);
void run_win_animation();
void set_password(AnimProperties*);
void run_fail_animation(AnimProperties*);
int8_t check_fail_animation(AnimProperties*);
int8_t check_key(AnimProperties*);

enum Anim{
	SET = 0, RESET = 1, TRUE = 2, FALSE = 3
};

int main(void)
{

	AnimProperties PropertiesObject;

	for(uint32_t i=0;i<10;i++)
	{
		PropertiesObject.password_input[i] = 0;
	}
	PropertiesObject.button_counter = 0;
	PropertiesObject.error_counter = 0;
	PropertiesObject.delay_counter = 0;
	set_password(&PropertiesObject);

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN);

	GPIOC->MODER &= ~(GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk | GPIO_MODER_MODE15_Msk);
	GPIOB->MODER &= ~(GPIO_MODER_MODE3_Msk | GPIO_MODER_MODE4_Msk | GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk);
	GPIOB->MODER |= (1 << GPIO_MODER_MODE3_Pos) | (1 << GPIO_MODER_MODE4_Pos) | (1 << GPIO_MODER_MODE5_Pos) | (1 << GPIO_MODER_MODE6_Pos);

	while(1)
	{

		if((GPIOC->IDR & GPIO_IDR_ID13) == 0 && PropertiesObject.password_input[PropertiesObject.button_counter] == 0)
		{
			PropertiesObject.password_input[PropertiesObject.button_counter] = 1;
			PropertiesObject.button_counter++;
			GPIOB->ODR |= (GPIO_ODR_OD3);
			while((GPIOC->IDR & GPIO_IDR_ID13) == 0);
			delay(200000);
		}

		if((GPIOC->IDR & GPIO_IDR_ID14) == 0 && PropertiesObject.password_input[PropertiesObject.button_counter] == 0)
		{
			PropertiesObject.password_input[PropertiesObject.button_counter] = 2;
			PropertiesObject.button_counter++;
			GPIOB->ODR |= (GPIO_ODR_OD4);
			while((GPIOC->IDR & GPIO_IDR_ID14) == 0);
			delay(200000);
		}

		if((GPIOC->IDR & GPIO_IDR_ID15) == 0 && PropertiesObject.password_input[PropertiesObject.button_counter] == 0)
		{
			PropertiesObject.password_input[PropertiesObject.button_counter] = 3;
			PropertiesObject.button_counter++;
			GPIOB->ODR |= (GPIO_ODR_OD5);
			while((GPIOC->IDR & GPIO_IDR_ID15) == 0);
			delay(200000);
		}

		if(PropertiesObject.button_counter == 10)
		{

			if(check_key(&PropertiesObject) == TRUE)
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

void run_win_animation()
{
	while(1)
	{
		GPIOB->ODR |= (GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6);
		delay(500000);
		GPIOB->ODR &= ~(GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6);
		delay(500000);

	}

}

void run_fail_animation(AnimProperties* PropertiesPtr)
{

	GPIOB->ODR &= ~(GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6);

	while(1)
	{
		if( (check_fail_animation(PropertiesPtr)) == RESET)
		{
			return;
		}

		GPIOB->ODR = (GPIO_ODR_OD3);
		delay(500000);
		GPIOB->ODR = (GPIO_ODR_OD4);
		delay(500000);
		GPIOB->ODR = (GPIO_ODR_OD5);
		delay(500000);
		GPIOB->ODR = (GPIO_ODR_OD6);
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
			GPIOB->ODR &= ~(GPIO_ODR_OD3 | GPIO_ODR_OD4 | GPIO_ODR_OD5 | GPIO_ODR_OD6);

			PropertiesPtr->delay_counter = 0;
			PropertiesPtr->error_counter++;
			PropertiesPtr->button_counter = 0;

			for(int i=0;i<10;i++)
			{
				PropertiesPtr->password_input[i] = 0;
			}

			EXTI->IMR1 |= EXTI_IMR1_IM13 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15;

			return RESET;
		}
	}

	return SET;
}

int8_t check_key(AnimProperties* PropertiesPtr)
{
	for(uint32_t i=0;i<10;i++)
	{
		if(PropertiesPtr->key[i] != PropertiesPtr->password_input[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

void set_password(AnimProperties* PropertiesPtr)
{
	int8_t temp_pass[10] = {1,1,1,2,2,2,3,3,3,1};
	for(uint32_t i=0;i<10;i++)
	{
		PropertiesPtr->key[i] = temp_pass[i];
	}
}
