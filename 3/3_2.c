#include <stdint.h>
#include "stm32g431xx.h"
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void view(int8_t num);
void setup_clock(void);

int8_t timer_state = 0;
int64_t number = 0;

int main(void)
{
	setup_clock();

	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN) | (RCC_AHB2ENR_GPIODEN);
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
	GPIOB->MODER &= ~(GPIO_MODER_MODE13_Msk | GPIO_MODER_MODE14_Msk | GPIO_MODER_MODE15_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) | (1 << GPIO_MODER_MODE3_Pos);
	GPIOB->MODER |= (0 << GPIO_MODER_MODE13_Pos) | (0 << GPIO_MODER_MODE14_Pos) | (0 << GPIO_MODER_MODE15_Pos);

	TIM2->PSC = 31999;
	TIM2->ARR = 2499;
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM2_IRQn);

	while(1)
	{
		if((GPIOB->IDR & GPIO_IDR_ID14) == 0)
		{
			TIM2->CNT = 0;
			TIM2->CR1 = TIM_CR1_CEN;

			while((GPIOB->IDR & GPIO_IDR_ID14) == 0) {};

			TIM2->CR1 &= ~TIM_CR1_CEN;

			if(timer_state == 0)
			{
				number++;
				if(number >= 16) number = 0;
			}

			else
			{
				timer_state = 0;
				number = 0;
			}

			view(number);
		}

	}
}


void view(int8_t num)
{
	GPIOD->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3);
	GPIOD->ODR |= (num & 0b00001111) << 0;
}

void TIM2_IRQHandler(void)
{
	timer_state=1;
	TIM2->CR1 &= ~TIM_CR1_CEN;
	TIM2->SR &= ~TIM_SR_UIF;
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
