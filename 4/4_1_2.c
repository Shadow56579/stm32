#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void setup_clock(void);
void delay(int64_t delay_size);
void setup_adc_dma( uint16_t *array_to_write_to );

int main(void)
{
	uint32_t current_value[2];

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIOEEN;

	GPIOE->MODER &= ~(GPIO_MODER_MODE0_Msk);
	GPIOE->MODER |= 1 << GPIO_MODER_MODE0_Pos;

	// Выбрать SYSCLK в качестве источника тактового сигнала для АЦП
	RCC->CCIPR |= (2U) << RCC_CCIPR_ADC12SEL_Pos;

	// Включить тактирование АЦП.
	RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;

	// Выйти из режима power-down.
	ADC2->CR &= ~(ADC_CR_DEEPPWD);

	// Включить внутренний преобразователь питания и подождать завершения.
	ADC2->CR |= ADC_CR_ADVREGEN;
	delay(10000);

	// Включить автоматическую калибровку АЦП и подождать пока она закончится.
	ADC2->CR |= ADC_CR_ADCAL;
	while ( ADC2->CR & ADC_CR_ADCAL ){}

	// Включить АЦП.
	ADC2->ISR &= ~ADC_ISR_ADRDY; // Очистить флаг ADRDY.
	ADC2->CR |= ADC_CR_ADEN;
	while ( !(ADC2->ISR & ADC_ISR_ADRDY) ){} // Подождать до включения.

	// Увеличить время считывания показания для нужного нам канала
	ADC2->SMPR1 |= 2 << ADC_SMPR1_SMP7_Pos
				   | 2 << ADC_SMPR1_SMP6_Pos;

	// Настроить канал, из которого мы будем считывать данные.
	// Канал №7,6 , всего каналов - 2
	ADC2->JSQR |= 7 << ADC_JSQR_JSQ2_Pos
				  | 6 << ADC_JSQR_JSQ1_Pos
				  | 1 << ADC_JSQR_JL_Pos;



	// Начать считывание данных из АЦП
	ADC2->CR |= ADC_CR_JADSTART;

	// Подождать до завершения измерения
	while ( !(ADC2->ISR & ADC_ISR_JEOS) ){}

	// Сбросить флаг завершения измерения
	ADC2->ISR &= ~ADC_ISR_JEOS;
	ADC2->ISR &= ~ADC_ISR_JEOC;

	// Прочитать данные из АЦП
	current_value[0] = ADC2->JDR1;
	current_value[1] = ADC2->JDR2;


	while(1)
	{
		delay(10000);
		ADC2->CR |= ADC_CR_JADSTART;
		while ( !(ADC2->ISR & ADC_ISR_JEOS) ){}
		ADC2->ISR &= ~ADC_ISR_JEOS;
		ADC2->ISR &= ~ADC_ISR_JEOC;
		current_value[0] = ADC2->JDR1;
		current_value[1] = ADC2->JDR2;
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
