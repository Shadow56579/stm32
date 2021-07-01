#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void setup_clock(void);
void delay(int64_t delay_size);

int main(void)
{
	setup_clock();//на 48Мгц

	while(1)
	{

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
	| 24 << RCC_PLLCFGR_PLLN_Pos
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

