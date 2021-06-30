#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void setup_clock(void);
void delay(int64_t delay_size);

int main(void)
{
	int8_t counter = 1;

	setup_clock();//на 64Мгц

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN;

	GPIOE->MODER &= ~(GPIO_MODER_MODE0_Msk);
	GPIOE->MODER |= (1 << GPIO_MODER_MODE0_Pos);

	//Настроить ножки на переменную функцию
	GPIOE->MODER &= ~(GPIO_MODER_MODE2_Msk);
	GPIOE->MODER |= (2 << GPIO_MODER_MODE2_Pos);
	GPIOE->AFR[0] |= 2 << GPIO_AFRL_AFSEL2_Pos;

	//Настройка таймера в режим ШИМ
	TIM3->PSC = 64000-1;
	TIM3->ARR = 999;
	TIM3->CCR1 = 500;//Установить коэффициент заполнения

	TIM3->CCMR1 |= TIM_CCMR1_OC1PE | 6 << TIM_CCMR1_OC1M_Pos;
	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC1P;//Включить 1 канал
	TIM3->CR1 |= TIM_CR1_ARPE;//Разрешить автоматическую перегрузку счетчика
	TIM3->CR1 |= TIM_CR1_CEN;//Включить таймер

	while(1)
	{
		if(counter == 1)
		{
			TIM3->PSC = 64000-1;
			counter++;
		}
		else if(counter == 2)
		{
			TIM3->PSC = 32000-1;
			counter++;
		}
		else if(counter == 3)
		{
			TIM3->PSC = 24000-1;
			counter++;
		}
		else if(counter == 4)
		{
			TIM3->PSC = 16000-1;
			counter = 1;
		}

		delay(10000000);
	}

}

void setup_clock()
{
	//Установить задержку на доступ к FLASH
	FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
	FLASH->ACR |= FLASH_ACR_LATENCY_2WS;

	//Включить HSE
	RCC->CR |= RCC_CR_HSEON;
	while ((RCC->CR & RCC_CR_HSERDY) != RCC_CR_HSERDY) {}

	//Сбросить значения регистра PLL
	RCC->PLLCFGR&=~(RCC_PLLCFGR_PLLR_Msk | RCC_PLLCFGR_PLLM_Msk |
	RCC_PLLCFGR_PLLN_Msk);

	//Настройка PLL
	RCC->PLLCFGR |= 0 << RCC_PLLCFGR_PLLR_Pos
	| RCC_PLLCFGR_PLLREN
	| 32 << RCC_PLLCFGR_PLLN_Pos
	| 1 << RCC_PLLCFGR_PLLM_Pos
	| RCC_PLLCFGR_PLLSRC_HSE;

	//Включить PLL
	RCC->CR |= RCC_CR_PLLON;

	//Подождать пока PLL включится
	while ((RCC->CR & RCC_CR_PLLRDY) != RCC_CR_PLLRDY){}

	//Использовать PLL в качестве источника SYSCLK
	RCC->CFGR |= RCC_CFGR_SW_Msk;
	RCC->CFGR &= ~(RCC_CFGR_SW_Msk ^ RCC_CFGR_SW_PLL);

	//Подождать пока источник частоты поменяется
	while ((RCC->CFGR & RCC_CFGR_SWS_PLL) != RCC_CFGR_SWS_PLL){}
}

void delay(int64_t delay_size)
{
	for(int64_t i;i<delay_size;i++);
}
