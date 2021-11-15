#include <stdint.h>
#include "Init.h"
#include "stm32g071xx.h"

void device_init(nrf24l01p* nrf24l01p_instance)
{
	timer1_init();
	timer2_init();
	led_init();
	drv_init();
	spi1_init();
	nrf24_basic_init(nrf24l01p_instance);
}

void spi1_init()
{
	//rcc
	RCC->APBENR2 |= RCC_APBENR2_SPI1EN;
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN | RCC_IOPENR_GPIOBEN;

	//gpio
	GPIOA->MODER &= ~(GPIO_MODER_MODE5_Msk | GPIO_MODER_MODE6_Msk | GPIO_MODER_MODE7_Msk);
	GPIOA->MODER |= (2 << GPIO_MODER_MODE5_Pos) | (2 << GPIO_MODER_MODE6_Pos) | (2 << GPIO_MODER_MODE7_Pos);
	GPIOA->AFR[0] |= (0 << GPIO_AFRL_AFSEL5_Pos) | (0 << GPIO_AFRL_AFSEL6_Pos) | (0 << GPIO_AFRL_AFSEL7_Pos);
	//CE&CSN
	GPIOB->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk);
	GPIOB->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos);

	//spi1
	SPI1->CR1 |= (0 << SPI_CR1_CPHA_Pos) | (0 << SPI_CR1_CPOL_Pos) | SPI_CR1_MSTR | (0 << SPI_CR1_BR_Pos)
			  | (0 << SPI_CR1_BIDIMODE_Pos) | SPI_CR1_SSM;
	SPI1->CR1  &= ~SPI_CR1_LSBFIRST;
	SPI1->CR2 |= (0b111 << SPI_CR2_DS_Pos) | SPI_CR2_FRXTH;
	SPI1->CR1 |= SPI_CR1_SSI;
	SPI1->CR1 |= SPI_CR1_SPE;
}

void led_init()
{
	//rcc
	RCC->IOPENR |= RCC_IOPENR_GPIODEN;

	//led
	GPIOD->MODER &= ~(GPIO_MODER_MODE0_Msk | GPIO_MODER_MODE1_Msk | GPIO_MODER_MODE2_Msk | GPIO_MODER_MODE3_Msk);
	GPIOD->MODER |= (1 << GPIO_MODER_MODE0_Pos) | (1 << GPIO_MODER_MODE1_Pos) | (1 << GPIO_MODER_MODE2_Pos) | (1 << GPIO_MODER_MODE3_Pos);
}

void drv_init()
{
	//rcc
	RCC->IOPENR |= RCC_IOPENR_GPIOCEN;

	//nSLEEP DRV8848
	GPIOC->MODER &= ~(GPIO_MODER_MODE6_Msk);
	GPIOC->MODER |= (1 << GPIO_MODER_MODE6_Pos);
	GPIOC->ODR |= GPIO_ODR_OD6;
}

void timer1_init()
{
	//rcc
	RCC->APBENR2 |= RCC_APBENR2_TIM1EN;
	RCC->IOPENR |= RCC_IOPENR_GPIOAEN;

	//gpio GPIOA(8-11) - PWM
	GPIOA->MODER &= ~(GPIO_MODER_MODE8_Msk | GPIO_MODER_MODE9_Msk | GPIO_MODER_MODE10_Msk | GPIO_MODER_MODE11_Msk);
	GPIOA->MODER |= (2 << GPIO_MODER_MODE8_Pos) | (2 << GPIO_MODER_MODE9_Pos) | (2 << GPIO_MODER_MODE10_Pos) | (2 << GPIO_MODER_MODE11_Pos);
	GPIOA->AFR[1] |= (2 << GPIO_AFRH_AFSEL8_Pos) | (2 << GPIO_AFRH_AFSEL9_Pos) | (2 << GPIO_AFRH_AFSEL10_Pos) | (2 << GPIO_AFRH_AFSEL11_Pos);

	//tim1
	TIM1->PSC = 0;
	TIM1->ARR = 500;
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;
	TIM1->CCR4 = 0;
	TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
	TIM1->CR1 |= TIM_CR1_ARPE;
	TIM1->CCMR1 |= TIM_CCMR1_OC1PE | 6 << TIM_CCMR1_OC1M_Pos | TIM_CCMR1_OC2PE | 6 << TIM_CCMR1_OC2M_Pos;
	TIM1->CCMR2	|= TIM_CCMR2_OC3PE | 6 << TIM_CCMR2_OC3M_Pos | TIM_CCMR2_OC4PE | 6 << TIM_CCMR2_OC4M_Pos;
	TIM1->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P;
	TIM1->BDTR |= TIM_BDTR_MOE;
	TIM1->CR1 |= TIM_CR1_CEN;
}

void timer2_init()
{
	//rcc
	RCC->APBENR1 |= RCC_APBENR1_TIM2EN;

	//tim2
	TIM2->PSC = 1599;
	TIM2->ARR = 4999;
	TIM2->DIER |= TIM_DIER_UIE;
	NVIC_EnableIRQ(TIM2_IRQn);
	TIM2->CR1 |= TIM_CR1_CEN;
}