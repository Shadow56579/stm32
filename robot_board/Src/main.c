#include <stdint.h>
#include "stm32g071xx.h"
#include "Init.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

nrf24l01p controller_nrf24 = {

		.ce_high = ce_high_function,
		.ce_low = ce_low_function,
		.csn_high = csn_high_function,
		.csn_low = csn_low_function,
		.spi_write_byte = spi1_write_function,

		.frequency_channel = 45,
		.power_output = nrf24_pa_max,
		.data_rate = nrf24_250_kbps,

		.payload_size_in_bytes = 6,
		.device_was_initialized = 0

};

uint8_t pipe1_rx_address[5] = {0x11, 0x22, 0x33, 0x44, 0x66}; // Must be 5 bytes long
uint8_t pip3_address = 0x77;
uint8_t pip5_address = 0x99;
uint8_t radio_controller_message[6] = {0, 0, 0, 0, 0, 0};
uint16_t device_connection = 0;

int main(void)
{
	device_init(&controller_nrf24);

	GPIOD->ODR |= GPIO_ODR_OD3;

	// This function should be called even if not pipe 1 is used to receive data because 4 first bytes of addresses of all pipes
	// are the same as pip1 and must set up through pipe1. Only last byte of every pipe rx address must be different.
	nrf24_enable_pipe1(&controller_nrf24, pipe1_rx_address);

	// Must be called for any nrf24l01+ instance to enable rx mode.
	nrf24_rx_mode(&controller_nrf24);

	while(1)
	{
		if(nrf24_is_new_data_availiable(&controller_nrf24))
		{
			nrf24_read_message(&controller_nrf24, radio_controller_message, sizeof(radio_controller_message));
			encode_buttons(radio_controller_message);
			encode_joystick((radio_controller_message[0] & 0x03),
							(radio_controller_message[0] & 0x0C) >> 2,
							(radio_controller_message[0] & 0x30) >> 4,
							(radio_controller_message[0] & 0xC0) >> 6,
							radio_controller_message);

			GPIOD->ODR |= GPIO_ODR_OD0;
			device_connection = 1;

		}
	}

}

void TIM2_IRQHandler(void)
{

	if(!device_connection)
	{
		reset_joystick();
		reset_buttons();

		GPIOD->ODR &= ~GPIO_ODR_OD0;
	}

	device_connection = 0;

	TIM2->SR &= ~TIM_SR_UIF;
}
