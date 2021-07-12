#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

uint16_t adc_current_data[2];

void delay(int64_t delay_size);

int main(void)
{

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN | RCC_AHB2ENR_GPIOEEN;
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM3EN | RCC_APB1ENR1_TIM2EN;

	GPIOE->MODER &= ~(GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
	GPIOE->MODER |= (2 << GPIO_MODER_MODE2_Pos) | (2 << GPIO_MODER_MODE3_Pos);
	GPIOE->AFR[0] |= (2 << GPIO_AFRL_AFSEL2_Pos) | (2 << GPIO_AFRL_AFSEL3_Pos);

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
	ADC2->SMPR1 |= 2 << ADC_SMPR1_SMP7_Pos
				   | 2 << ADC_SMPR1_SMP6_Pos;

	// Настроить канал, из которого мы будем считывать данные.
	// Канал №7,6 , всего каналов - 2
	ADC2->JSQR |= 7 << ADC_JSQR_JSQ2_Pos
				  | 6 << ADC_JSQR_JSQ1_Pos
				  | 1 << ADC_JSQR_JL_Pos;

	//Настройка ШИМ
	TIM3->PSC = 0;
	TIM3->ARR = 399;
	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE | 6 << TIM_CCMR1_OC1M_Pos
			    | TIM_CCMR1_OC2PE | 6 << TIM_CCMR1_OC2M_Pos;
	TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC1P
			   | TIM_CCER_CC2E | TIM_CCER_CC2P;//Включить 1 канал
	TIM3->CR1 |= TIM_CR1_ARPE;//Разрешить автоматическую перегрузку счетчика

	//Настройка 2-го таймера
	TIM2->PSC = 199;
	TIM2->ARR = 499;
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM2_IRQn);

	TIM2->CR1 |= TIM_CR1_CEN;//Включить таймер 2
	TIM3->CR1 |= TIM_CR1_CEN;//Включить таймер 3


	while(1)
	{
		// Начать считывание данных из АЦП
		ADC2->CR |= ADC_CR_JADSTART;

		// Подождать до завершения измерения
		while ( !(ADC2->ISR & ADC_ISR_JEOS) ){}

		// Сбросить флаг завершения измерения
		ADC2->ISR &= ~ADC_ISR_JEOS;
		ADC2->ISR &= ~ADC_ISR_JEOC;

		adc_current_data[0] = ADC2->JDR1;
		adc_current_data[1] = ADC2->JDR2;
	}

}

void TIM2_IRQHandler(void)
{
	uint32_t temp;

	temp = (uint32_t) ((adc_current_data[0] * 100) / 4096);
	TIM3->CCR1 = (uint32_t) ((temp * 400) / 100);

	temp = (uint32_t) ((adc_current_data[1] * 100) / 4096);
	TIM3->CCR2 = (uint32_t) ((temp * 400) / 100);

	TIM2->SR &= ~TIM_SR_UIF;
}

void delay(int64_t delay_size)
{
	for(int64_t i;i<delay_size;i++);
}
