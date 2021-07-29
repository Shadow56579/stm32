#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void delay(int64_t delay_size);

int main(void)
{
	uint16_t adc_current_data;

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIOEEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

	GPIOE->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk
					 | GPIO_MODER_MODE3_Msk);
	GPIOE->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos)
			         | (1 << GPIO_MODER_MODE2_Pos) | (1 << GPIO_MODER_MODE3_Pos);

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
	ADC2->SMPR1 |= 2 << ADC_SMPR1_SMP6_Pos;

	// Настроить канал, из которого мы будем считывать данные.
	// Канал №6 , всего каналов - 1
	ADC2->SQR1 |= 6 << ADC_SQR1_SQ1_Pos | 0 << ADC_SQR1_L_Pos;

	while(1)
	{
		// Начать считывание данных из АЦП
		ADC2->CR |= ADC_CR_ADSTART;

		// Подождать до завершения измерения
		while ( !(ADC2->ISR & ADC_ISR_EOS) ){}

		// Сбросить флаг завершения измерения
		ADC2->ISR |= ADC_ISR_EOS;

		adc_current_data = ADC2->DR;

		GPIOE->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3);

		if(adc_current_data <= 1000)
		{
			GPIOE->ODR &= ~(GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3);
		}
		else if(adc_current_data > 1000 && adc_current_data <= 2000)
		{
			GPIOE->ODR |= GPIO_ODR_OD0;
		}
		else if(adc_current_data > 2000 && adc_current_data <= 3000)
		{
			GPIOE->ODR |= GPIO_ODR_OD0 | GPIO_ODR_OD1;
		}
		else if(adc_current_data > 3000 && adc_current_data <= 3700)
		{
			GPIOE->ODR |= GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2;
		}
		else if(adc_current_data > 3700)
		{
			GPIOE->ODR |= GPIO_ODR_OD0 | GPIO_ODR_OD1 | GPIO_ODR_OD2 | GPIO_ODR_OD3;
		}
	}

}

void delay(int64_t delay_size)
{
	for(int64_t i;i<delay_size;i++);
}
