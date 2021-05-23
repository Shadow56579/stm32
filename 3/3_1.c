#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void delay(int64_t delay_time);
void setup_clock(void);

int8_t time_state = 0;

int main(void)
{
	setup_clock();

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOEEN);
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);

	GPIOE->MODER &= ~(GPIO_MODER_MODE1_Msk);
	GPIOE->MODER |= (1 << GPIO_MODER_MODE1_Pos);

	//Разрешение прерывания по переполнению
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM2_IRQn);

	while(1)
	{
		GPIOE->ODR = GPIO_ODR_OD1;
		delay(5000);
		GPIOE->ODR = 0;
		delay(5000);
	}

	return 0;
}

void delay(int64_t delay_time)
{
	int64_t temp = 64000000*delay_time/1000;

	TIM2->ARR = delay_time - 1;
	TIM2->PSC = 64000000/temp - 1;

	TIM2->CR1 |= TIM_CR1_CEN;

	while(1)
	{
		if(time_state == 1)
		{
			time_state = 0;
			break;
		}
	}

	TIM2->CR1 &= ~TIM_CR1_CEN;
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

void TIM2_IRQHandler(void)
{
	time_state = 1;
	TIM2->CR1 &= ~TIM_CR1_CEN;
	TIM2->SR &= ~TIM_SR_UIF;
}
