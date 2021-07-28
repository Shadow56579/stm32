#include <stdint.h>
#include "stm32g431xx.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

void delay(int64_t delay_size);
void setup_adc_dma( uint16_t *array_to_write_to );

uint16_t adc_data[2] = { 0 };
uint16_t current_adc_value;

int main(void)
{

	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

	// Выбрать SYSCLK в качестве источника тактового сигнала для АЦП
	RCC->CCIPR |= (2U) << RCC_CCIPR_ADC12SEL_Pos;

	// Включить тактирование АЦП.
	RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN;

	// Выйти из режима power-down.
	ADC2->CR &= ~(ADC_CR_DEEPPWD);

	// Включить внутренний преобразователь питания и подождать, пока он запустится.
	ADC2->CR |= ADC_CR_ADVREGEN;
	delay(10000);

	// Включить автоматическую калибровку АЦП и подождать пока она закончится.
	ADC2->CR |= ADC_CR_ADCAL;
	while ( ADC2->CR & ADC_CR_ADCAL );

	// Включить АЦП.
	ADC2->ISR |= ADC_ISR_ADRDY; // Очистить флаг ADRDY.
	ADC2->CR |= ADC_CR_ADEN;

	while ( !(ADC2->ISR & ADC_ISR_ADRDY) ){} // Подождать до включения.
	// Увеличить время считывания показания для нужного нам канала
	ADC2->SMPR1 |= 2 << ADC_SMPR1_SMP7_Pos
				| 2 << ADC_SMPR1_SMP6_Pos;

	// Настроить канал, из которого мы будем считывать данные.
	ADC2->SQR1 |= 7 << ADC_SQR1_SQ2_Pos
			   | 6 << ADC_SQR1_SQ1_Pos
			   | 1 << ADC_SQR1_L_Pos;

	// Включить работу с DMA в АЦП и настроить DMA.
	ADC2->CFGR |= ADC_CFGR_DMAEN | ADC_CFGR_DMACFG;

	setup_adc_dma(adc_data);
	
	ADC2->CR |= ADC_CR_ADSTART;

	while ( !(ADC2->ISR & ADC_ISR_EOS) ){}

	ADC2->ISR |= ADC_ISR_EOS;

	while(1)
	{


	}


}

void delay(int64_t delay_size)
{
	for(int64_t i;i<delay_size;i++);
}

void setup_adc_dma( uint16_t *array_to_write_to )
{
	// Включить тактирование DMA
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMAMUX1EN;

	// Настроить адрес откуда будет происходить чтение
	DMA1_Channel1->CPAR = (uint32_t)&(ADC2->DR);

	// Настроить адрес куда будет происходить записью надо напрямую преобразовать
	// адрес в число типа uin32_t, так как все адреса - 32 значения.
	DMA1_Channel1->CMAR = (uint32_t)array_to_write_to;

	// Настроить количество значений, которые мы хотим считать.
	DMA1_Channel1->CNDTR = 2;

	// Настроить канал DMA: размер данных 16 бит источник и приемник, циклический
	// режи, увеличивать указатель при записи.
	DMA1_Channel1->CCR |= 1 << DMA_CCR_MSIZE_Pos | 1 << DMA_CCR_PSIZE_Pos
					   | DMA_CCR_MINC | DMA_CCR_CIRC;

	// Настроить мультиплексор каналов DMA
	DMAMUX1_Channel0->CCR |= (36 << DMAMUX_CxCR_DMAREQ_ID_Pos);

	// Включить DMA
	DMA1_Channel1->CCR |= DMA_CCR_EN;
}

