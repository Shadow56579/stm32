#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void delay(int64_t delay_size);

int main(void)
{
	uint32_t current_value;

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
	ADC2->ISR |= ADC_ISR_ADRDY; // Очистить флаг ADRDY.
	ADC2->CR |= ADC_CR_ADEN;
	while ( !(ADC2->ISR & ADC_ISR_ADRDY) ){} // Подождать до включения.

	// Увеличить время считывания показания для нужного нам канала
	ADC2->SMPR1 |= 2 << ADC_SMPR1_SMP7_Pos;

	// Настроить канал, из которого мы будем считывать данные.
	// Канал №6, всего каналов - 1
	ADC2->SQR1 |= 7 << ADC_SQR1_SQ1_Pos | 0 << ADC_SQR1_L_Pos;

	// Начать считывание данных из АЦП
	ADC2->CR |= ADC_CR_ADSTART;

	// Подождать до завершения измерения
	while ( !(ADC2->ISR & ADC_ISR_EOS) ){}

	// Сбросить флаг завершения измерения
	ADC2->ISR |= ADC_ISR_EOS;

	// Прочитать данные из АЦП
	current_value = ADC2->DR;

	while(1)
	{
		delay(1000000);
		GPIOE->ODR ^= GPIO_ODR_OD0;

		// Начать считывание данных из АЦП
		ADC2->CR |= ADC_CR_ADSTART;

		// Подождать до завершения измерения
		while ( !(ADC2->ISR & ADC_ISR_EOS) ){}

		// Сбросить флаг завершения измерения
		ADC2->ISR |= ADC_ISR_EOS;

		current_value = ADC2->DR;
	}

}

void delay(int64_t delay_size)
{
	for(int64_t i;i<delay_size;i++);
}
