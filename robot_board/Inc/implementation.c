#include "implementation.h"

static void delay(uint32_t dl)
{
	for(int i = 0;i<dl;i++) {};
}


uint8_t spi1_write_function( const uint8_t byte_to_be_sent )
{
    // Wait until transmit buffer is empty
    while ( (SPI1->SR & SPI_SR_TXE) != SPI_SR_TXE ) {};

    // Write single byte into the Data Register with single byte access
    *((volatile uint8_t*)&SPI1->DR) = byte_to_be_sent;

    // Wait until answer will appear in RX buffer
    while ( ((SPI1->SR & SPI_SR_RXNE) != SPI_SR_RXNE) ){}

    // Return value from RX buffer
    return SPI1->DR;
}

void ce_high_function(void)
{
	GPIOB->ODR |= GPIO_ODR_OD0;
}

void ce_low_function(void)
{
	GPIOB->ODR &= ~(GPIO_ODR_OD0);
}

void csn_high_function(void)
{
	GPIOB->ODR |= GPIO_ODR_OD1;
}

void csn_low_function(void)
{
	GPIOB->ODR &= ~(GPIO_ODR_OD1);
}

void encode_buttons(uint8_t radio_controller_message[6])
{

	for(uint8_t i = 1;i<=2;i++)
	{
		uint8_t temp = radio_controller_message[1] & (1 << i-1);
		if(temp != 0)
		{
			GPIOD->ODR |= (0x1UL << i);
		}
		else
		{
			GPIOD->ODR &= ~(0x1UL << i);
		}
	}

}

void encode_joystick(uint8_t right_joystick_top_bottom, uint8_t right_joystick_left_right,
					 uint8_t left_joystick_left_right, uint8_t left_joystick_top_bottom,
					 uint8_t radio_controller_message[6])
{
	uint8_t button_state = 0;

	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;
	TIM1->CCR4 = 0;
	delay(1000);

	if((radio_controller_message[1] & (1 << 5)) != 0)
	{
		button_state = 2;
	}
	else if((radio_controller_message[1] & (1 << 4)) != 0)
	{
		button_state = 1;
	}
	else
	{
		button_state = 0;
	}

	//вперед
	if(right_joystick_top_bottom == 1 && left_joystick_left_right == 0)
	{
		TIM1->CCR1 = 300 + 90*button_state;
		TIM1->CCR4 = 300 + 90*button_state;
	}
	//назад
	else if(right_joystick_top_bottom == 2 && left_joystick_left_right == 0)
	{
		TIM1->CCR2 = 300 + 90*button_state;
		TIM1->CCR3 = 300 + 90*button_state;
	}
	//вперед-направо
	else if(left_joystick_left_right == 2 && right_joystick_top_bottom == 1)
	{
		TIM1->CCR1 = 300 + 90*button_state;
		TIM1->CCR4 = 200 + 90*button_state;
	}
	//вперед-налево
	else if(left_joystick_left_right == 1 && right_joystick_top_bottom == 1)
	{
		TIM1->CCR1 = 200 + 90*button_state;
		TIM1->CCR4 = 300 + 90*button_state;
	}
	//назад-направо
	else if(left_joystick_left_right == 2 && right_joystick_top_bottom == 2)
	{
		TIM1->CCR2 = 300 + 90*button_state;
		TIM1->CCR3 = 200 + 90*button_state;
	}
	//назад-налево
	else if(left_joystick_left_right == 1 && right_joystick_top_bottom == 2)
	{
		TIM1->CCR2 = 200 + 90*button_state;
		TIM1->CCR3 = 300 + 90*button_state;
	}
	//направо
	else if(left_joystick_left_right == 2)
	{
		TIM1->CCR1 = 300 + 90*button_state;
		TIM1->CCR3 = 300 + 90*button_state;
	}
	//налево
	else if(left_joystick_left_right == 1)
	{
		TIM1->CCR2 = 300 + 90*button_state;
		TIM1->CCR4 = 300 + 90*button_state;
	}

}

void reset_joystick(void)
{
	TIM1->CCR1 = 0;
	TIM1->CCR2 = 0;
	TIM1->CCR3 = 0;
	TIM1->CCR4 = 0;
}

void reset_buttons(void)
{
	GPIOD->ODR &= ~(GPIO_ODR_OD1 | GPIO_ODR_OD2);
}

