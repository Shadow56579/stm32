#include <nrf24l01p_impi.h>

// ******************* Function ******************* //
/*
	@brief Sets up nrf24l01+ with parameters specified in the structure of particular instance.
		Doesn't power up device. Doesn't enable any mode. Should be called at least one time for every nrf24l01+ instance.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called

	@return mistake code or 0 if no mistakes were found
*/
// ************************************************ //
uint32_t nrf24_basic_init(nrf24l01p *nrf24_instance)
{
	// Check if the particular device was initialized properly
	uint32_t declaration_mistake_code = nrf24_check_declarations(nrf24_instance);
	if ( declaration_mistake_code )
	{
		// Function is immediately stopped
		return declaration_mistake_code;
	}

	// Check if the particular device is at least in power-down mode. It takes 100ms for the device to load into the power-down from the power on.
	// So if the function is called immediately device could not be ready yet
	uint32_t checks = 0;
	while ( nrf24_check_if_alive(nrf24_instance) )
	{
		// Small dummy delay. Efficiency will depend on MCU clock speed
		for (uint32_t dummy_delay = 0; dummy_delay < 1000; dummy_delay++){}

		checks += 1;

		// If after 10 checks device is still not responding return mistake code
		if ( checks == 10 )
		{
			return NRF24_DEVICE_IS_NOT_CONNECTED;
		}
	}

	// To start SPI transmission high to low front should be detected. So we should prepare lone setting it high
	nrf24_instance->csn_high();

	// Go to standby-1 mode in case init is called to re-setup device
	nrf24_instance->ce_low();

	// This function will return mistake code if mistake occurs. If more then one mistake occurs only code of the last one will be returned
	uint32_t mistake_code = 0;

	// Default config setup - disable all interrupts, enable CRC with 1 byte encoding, TX mode with NO power up
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_CONFIG);
	nrf24_instance->spi_write_byte(NRF24_INTERRUPTS_MASK | NRF24_EN_CRC | NRF24_CRCO);
	nrf24_instance->csn_high();

	// Checking frequency channel. Change to valid value if wrong and throw mistake. Then write value to NRF
	if ( nrf24_instance->frequency_channel < 1 )
	{
		nrf24_instance->frequency_channel = 1;
		mistake_code = NRF24_WRONG_CHANNEL_FREQUENCY;
	}
	else if ( nrf24_instance->frequency_channel > 124 )
	{
		nrf24_instance->frequency_channel = 124;
		mistake_code = NRF24_WRONG_CHANNEL_FREQUENCY;
	}
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_RF_CH);
	nrf24_instance->spi_write_byte(nrf24_instance->frequency_channel);
	nrf24_instance->csn_high();

	// Set up device data rate and power output
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_RF_SETUP);
	nrf24_instance->spi_write_byte(nrf24_instance->data_rate | nrf24_instance->power_output);
	nrf24_instance->csn_high();

	// Reset all interrupt flags in case setup is called more than one time to dynamically change parameters of nrf24l01+ instance
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_STATUS);
	nrf24_instance->spi_write_byte(NRF24_INTERRUPTS_MASK);
	nrf24_instance->csn_high();

	// If data rate is 250kbps, set retransmission delay to 750us (datasheet recommended is 500us at 250kbps) and retransmission count to 2
	if (nrf24_instance->data_rate == nrf24_250_kbps)
	{
		nrf24_instance->csn_low();
		nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_SETUP_RETR);
		nrf24_instance->spi_write_byte(0x2 | nrf24_wait_750_us);
		nrf24_instance->csn_high();
	}

	// Check if payload size is correct. If needed change value and throw mistake. Then write value for pipe 0
	if ( nrf24_instance->payload_size_in_bytes < 0 )
	{
		nrf24_instance->payload_size_in_bytes = 0;
		mistake_code = NRF24_WRONG_PAYLOAD_SIZE;
	}
	else if ( nrf24_instance->payload_size_in_bytes > 32 )
	{
		nrf24_instance->payload_size_in_bytes = 32;
		mistake_code = NRF24_WRONG_PAYLOAD_SIZE;
	}
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_RX_PW_P0);
	nrf24_instance->spi_write_byte(nrf24_instance->payload_size_in_bytes);
	nrf24_instance->csn_high();

	// Write same payload size for pipe 1
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_RX_PW_P1);
	nrf24_instance->spi_write_byte(nrf24_instance->payload_size_in_bytes);
	nrf24_instance->csn_high();

	// Device was initialized
	nrf24_instance->device_was_initialized = 1;

	return mistake_code;
}


// ******************* Function ******************* //
/*
	@brief Checks if particular devices responses, if not returns mistake code.

	@note Reads RF_CH register (which is never equals 0). If response data equals 0 - device is not connected, returns mistake code.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called.

	@return mistake code or 0 if no mistakes were found.
*/
// ************************************************ //
uint32_t nrf24_check_if_alive(nrf24l01p *nrf24_instance)
{
	nrf24_instance->csn_low();

	nrf24_instance->spi_write_byte(NRF24_RF_CH);

	if ( nrf24_instance->spi_write_byte(NRF24_NOP) )
	{
		nrf24_instance->csn_high();
		return 0;
	}

	nrf24_instance->csn_high();
	return NRF24_DEVICE_IS_NOT_CONNECTED;
}

// ******************* Function ******************* //
/*
	@brief Checks if all function pointers of particular nrf24l01+ instance struct were initialized. If not returns mistake code.
		Those mistakes should be handled as critical, so any function calling this one should immediately return mistake code.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called

	@return first found mistake code or 0 if no mistakes were found
 */
// ************************************************ //
uint32_t nrf24_check_declarations(nrf24l01p *nrf24_instance)
{
	if ( nrf24_instance->ce_high == 0 )
	{
		return NRF24_CE_HIGH_FUNCTION_IS_MISSING;
	}

	if ( nrf24_instance->ce_low == 0 )
	{
		return NRF24_CE_LOW_FUNCTION_IS_MISSING;
	}

	if ( nrf24_instance->csn_high == 0 )
	{
		return NRF24_CSN_HIGH_FUNCTION_IS_MISSING;
	}

	if ( nrf24_instance->csn_low == 0 )
	{
		return NRF24_CSN_LOW_FUNCTION_IS_MISSING;
	}

	if ( nrf24_instance->spi_write_byte == 0 )
	{
		return NRF24_SPI_WRITE_FUNCTION_IS_MISSING;
	}

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Changes device setup to TX, powers device up. Device is ready to transmit data.

	@note Doesn't pull CE logic high for power efficiency - device stays in standby-1. CE is used only in data transmission.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called

	@return first found mistake code or 0 if no mistakes were found
 */
// ************************************************ //
uint32_t nrf24_tx_mode(nrf24l01p *nrf24_instance)
{
	// Check if device was initialized
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	// Go to standby-1 mode if CE = 1 to safely switch modes
	nrf24_instance->ce_low();

	// Read current config state
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_R_REGISTER | NRF24_CONFIG);
	uint8_t current_register_state = nrf24_instance->spi_write_byte(NRF24_NOP);
	nrf24_instance->csn_high();


	// Add power up to current config
	current_register_state &= ~NRF24_PRIM_RX;

	// Write new config state with POWER_UP
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_CONFIG);
	nrf24_instance->spi_write_byte(current_register_state | NRF24_PWR_UP);
	nrf24_instance->csn_high();

	// Clear TX FIFO if it was not empty
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_FLUSH_TX);
	nrf24_instance->csn_high();

	// Reset all interrupt flags
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_STATUS);
	nrf24_instance->spi_write_byte(NRF24_INTERRUPTS_MASK);
	nrf24_instance->csn_high();

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Changes device settings to RX, powers up the device, start device. Device is ready to receive data.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called

	@return first found mistake code or 0 if no mistakes were found
 */
// ************************************************ //
uint32_t nrf24_rx_mode(nrf24l01p *nrf24_instance)
{
	// Check if device was initialized
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	// Go to standby-1 mode
	nrf24_instance->ce_low();

	// Read current config state
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_R_REGISTER | NRF24_CONFIG);
	uint8_t current_register_state = nrf24_instance->spi_write_byte(NRF24_NOP);
	nrf24_instance->csn_high();

	// Add power up to current config
	current_register_state |= NRF24_PRIM_RX;

	// Write new config state with power up
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_CONFIG);
	nrf24_instance->spi_write_byte(current_register_state | NRF24_PWR_UP);
	nrf24_instance->csn_high();

	// Clear RX FIFO if it was not empty
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_FLUSH_RX);
	nrf24_instance->csn_high();

	// Reset all interrupt flags
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_STATUS);
	nrf24_instance->spi_write_byte(NRF24_INTERRUPTS_MASK);
	nrf24_instance->csn_high();

	// NRF should always be in RX mode not to miss data
	nrf24_instance->ce_high();

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Sets new TX and RX pipe 0 addresses. Address should be 5 bytes long. Powers device down, So after this function either nrf24_rx_mode or nrf24_tx_mode should be called.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called
	@param [in] new_tx_address[5] - array which contains new tx address. Programmer must make sure, that array is exactly 5 elements long.

	@return first found mistake code or 0 if no mistakes were found

	@note Addresses should be written in reverse order, so that they can be represented as they were stored in the array.
 */
// ************************************************ //
uint32_t nrf24_set_tx_address(nrf24l01p *nrf24_instance, const uint8_t new_tx_address[5])
{
	// Check if device was initialized
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	// if device was in RX or TX mode it should be reset to standby-1
	nrf24_instance->ce_low();

	// Change address of pipe 0 which is used for reception  of ACK
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_RX_ADDR_P0);
	for(uint32_t i = 0; i < 5; ++i)
	{
		nrf24_instance->spi_write_byte(new_tx_address[4-i]);
	}
	nrf24_instance->csn_high();

	// Change TX address
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_TX_ADDR);
	for(uint32_t i = 0; i < 5; ++i)
	{
		nrf24_instance->spi_write_byte(new_tx_address[4-i]);
	}
	nrf24_instance->csn_high();

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Sets new RX address for pipe 1. Address should be 5 bytes long. 4 first bytes are used as base for addresses of pipes 2 to 5.
		Powers device down, So after this function either nrf24_rx_mode or nrf24_tx_mode should be called.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called
	@param [in] pipe_address[] - new base address of pipes 1-5 (first 4 bytes) and unique byte for pipe 1 (last byte). user should make sure, that array has a length of 5 bytes.

	@return first found mistake code or 0 if no mistakes were found
 */
// ************************************************ //

uint32_t nrf24_enable_pipe1(nrf24l01p *nrf24_instance, uint8_t pipe_address[])
{
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	// Stop RX mode if was on
	nrf24_instance->ce_low();

	// Write new pipe 1 address
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_RX_ADDR_P1);
	for(uint32_t i = 0; i < 5; ++i)
	{
		nrf24_instance->spi_write_byte(pipe_address[4-i]);
	}
	nrf24_instance->csn_high();

	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_R_REGISTER | NRF24_EN_RXADDR);
	uint8_t register_state = nrf24_instance->spi_write_byte(NRF24_NOP);
	nrf24_instance->csn_high();

	register_state |= 0x02;

	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_EN_RXADDR);
	nrf24_instance->spi_write_byte(register_state);
	nrf24_instance->csn_high();

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Sets new RX address for pipe from 2 to 5 and enables corresponding pipe. Use base from pipe 4, so only one byte changes.
		Powers device down, So after this function either nrf24_rx_mode or nrf24_tx_mode should be called.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called.
	@param [in] pipe_number - number of the pipe to be enabled.
	@param [in] pipe_address_last_byte - last address byte for the particular pipe.

	@return first found mistake code or 0 if no mistakes were found
 */
// ************************************************ //
uint32_t nrf24_enable_pipe2_5(nrf24l01p *nrf24_instance, uint32_t pipe_number, uint8_t pipe_address_last_byte)
{
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	// Stop RX mode if was on
	nrf24_instance->ce_low();

	// Wrong pipe number is critical mistake
	if ( pipe_number < 2 || pipe_number > 5 )
	{
		return NRF24_WRONG_PIPE_NUMBER;
	}

	// Set new address
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | (NRF24_RX_ADDR_P0 + pipe_number));
	nrf24_instance->spi_write_byte(pipe_address_last_byte);
	nrf24_instance->csn_high();

	// Set payload size
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | (NRF24_RX_PW_P0 + pipe_number));
	nrf24_instance->spi_write_byte(nrf24_instance->payload_size_in_bytes);
	nrf24_instance->csn_high();

	// Enable pipe by reading current state and adding new bit into it
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_R_REGISTER | NRF24_EN_RXADDR);
	uint8_t current_register_state = nrf24_instance->spi_write_byte(NRF24_NOP);
	nrf24_instance->csn_high();

	current_register_state |= 1<<pipe_number;

	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_EN_RXADDR);
	nrf24_instance->spi_write_byte(current_register_state);
	nrf24_instance->csn_high();

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Sends message with either acknowledgment or not.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called.
	@param [in] payload - pointer to the payload (array, containing message to be sent).
	@param [in] payload_size_in_bytes - payload size in bytes.
	@param [in] should send ack - flag that determines if the message should be sent with or without acknowledgment.

	@return first found mistake code or 0 if no mistakes were found
 */
// ************************************************ //
uint32_t nrf24_send_message(nrf24l01p *nrf24_instance,  void *payload, uint32_t payload_size_in_bytes, int32_t should_flush_tx, int32_t should_send_ack)
{
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	if (should_flush_tx) //Flush Tx FIFO if needed
	{
		// Clear TX FIFO
		nrf24_instance->csn_low();
		nrf24_instance->spi_write_byte(NRF24_FLUSH_TX);
		nrf24_instance->csn_high();
	}

	uint32_t mistake_code = 0;

	if ( payload_size_in_bytes > nrf24_instance->payload_size_in_bytes )
	{
		payload_size_in_bytes = nrf24_instance->payload_size_in_bytes;
		mistake_code = NRF24_WRONG_MESSAGE_SIZE;
	}

	// Reset all interrupt flags
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_STATUS);
	nrf24_instance->spi_write_byte(NRF24_INTERRUPTS_MASK);
	nrf24_instance->csn_high();

	// Data will be sent byte by byte
	uint8_t *current_byte_to_send = payload;

	// If payload input is shorter then actual payload, not specified bytes will be sent as 0.
	uint32_t amount_of_zeros_requered = nrf24_instance->payload_size_in_bytes - payload_size_in_bytes;

	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(should_send_ack == 1 ? NRF24_W_TX_PAYLOAD : W_TX_PAYLOAD_NO_ACK);

	// Send data
	for ( uint32_t i = 0; i < payload_size_in_bytes; ++i )
	{
		nrf24_instance->spi_write_byte(*current_byte_to_send);
		current_byte_to_send++;
	}

	// Fill empty space with 0 if needed.
	for ( uint32_t i = 0; i < amount_of_zeros_requered; ++i )
	{
		nrf24_instance->spi_write_byte(0);
	}
	nrf24_instance->csn_high();

	// Send device into the Tx mode to send one payload
	nrf24_instance->ce_high();
	for ( int i = 0; i < 500; ++i ){}
	nrf24_instance->ce_low();

	return mistake_code;
}

// ******************* Function ******************* //
/*
	@brief Checks if new data is available. If so return the number of the pipe which received message.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called

	@return number of the pipe which received message or 0 if no message was received.
 */
// ************************************************ //
uint8_t nrf24_is_new_data_availiable(nrf24l01p *nrf24_instance)
{
	// IF device is not initialized will return no data
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return 0;
	}

	nrf24_instance->csn_low();
	uint8_t current_device_status = nrf24_instance->spi_write_byte(NRF24_R_REGISTER | NRF24_FIFO_STATUS);
	uint8_t current_fifo_status = nrf24_instance->spi_write_byte(NRF24_NOP);
	nrf24_instance->csn_high();

	if((current_fifo_status & 0x01) == 0)
	{

		return (current_device_status &0x0E) >> 1;
	}

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Saves last received message into the input array.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called
	@param [in] payload_storage - pointer to the array into which the message should be saved.
	@param [in] payload_size_in_bytes - payload size in bytes.

	@return number of the pipe which received message or 0 if no message was received.
 */
// ************************************************ //
uint32_t nrf24_read_message(nrf24l01p *nrf24_instance, void *payload_storage, uint32_t payload_size_in_bytes)
{
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	uint8_t mistake_code = 0;

	uint8_t *current_byte_to_be_read = payload_storage;

	// Check for proper payload size
	if(payload_size_in_bytes > nrf24_instance->payload_size_in_bytes)
	{
		payload_size_in_bytes = nrf24_instance->payload_size_in_bytes;
		mistake_code = NRF24_WRONG_PAYLOAD_SIZE;
	}
	if(payload_size_in_bytes < 0)
	{
		return NRF24_WRONG_PAYLOAD_SIZE;
	}

	// Read data stored in RX FIFO
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_R_RX_PAYLOAD);
	for (uint32_t i = 0; i < payload_size_in_bytes; ++i)
	{
		*current_byte_to_be_read = nrf24_instance->spi_write_byte(NRF24_NOP);
		current_byte_to_be_read++;
	}

	uint32_t zeros_to_be_read = nrf24_instance->payload_size_in_bytes - payload_size_in_bytes;
	for(uint32_t i = 0; i < zeros_to_be_read; ++i)
	{
		nrf24_instance->spi_write_byte(NRF24_NOP);
	}
	nrf24_instance->csn_high();

	// Clear new RX data interrupt flag
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_STATUS);
	nrf24_instance->spi_write_byte(NRF24_MASK_RX_DR);
	nrf24_instance->csn_high();

	return mistake_code;
}

// ******************* Function ******************* //
/*
	@brief Enables interrupts with 1 in related input parameters, disables interrupts with 0. So to disable all interrupts call function with all 0 as inputs.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called
	@param [in] enable_rx_dr - flag to enable RX interrupt
	@param [in] enable_tx_dr - flag to enable TX interrupt
	@param [in] enable_max_rt - flag to enable "maximum number of retransmissions exceeded" interrupt

	@return mistake code or 0 if no mistakes were found
 */
// ************************************************ //
uint32_t nrf24_enable_interrupts(nrf24l01p *nrf24_instance,	uint32_t enable_rx_dr, uint32_t enable_tx_ds, uint32_t enable_max_rt)
{
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	uint8_t interrupts_to_be_enabled = 0;

	if ( enable_rx_dr == 0 )
	{
		interrupts_to_be_enabled |= 0x40;
	}
	if ( enable_tx_ds == 0 )
	{
		interrupts_to_be_enabled |= 0x20;
	}
	if ( enable_max_rt == 0 )
	{
		interrupts_to_be_enabled |= 0x10;
	}

	// Read current config state
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_R_REGISTER | NRF24_CONFIG);
	uint8_t current_register_state = nrf24_instance->spi_write_byte(NRF24_NOP);
	nrf24_instance->csn_high();

	// Remove interrupt masking from current config state
	current_register_state &= ~NRF24_INTERRUPTS_MASK;
	current_register_state |= interrupts_to_be_enabled;

	// Write new config state
	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_CONFIG);
	nrf24_instance->spi_write_byte(current_register_state);
	nrf24_instance->csn_high();

	return 0;
}

// ******************* Function ******************* //
/*
	@brief Returns interrupt flags in raw state. Clears interrupts flags.

	@note Interrupts flags in registers are in bits number 4, 5 and 6 (MAX_RT, TX_DS, RX_DR respectively). So return value can be 0x00 or 0x10 to 0x70.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called.

	@return NRF24_STATUS register interrupts bits.
 */
// ************************************************ //
uint8_t nrf24_get_interrupts_status(nrf24l01p * nrf24_instance)
{

	nrf24_instance->csn_low();
	uint8_t interrupt_status = nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_STATUS) & 0xF0;
	nrf24_instance->spi_write_byte(NRF24_INTERRUPTS_MASK);
	nrf24_instance->csn_high();

	return interrupt_status;
}

// ******************* Function ******************* // (V)
/*
	@brief Sets new values for retransmit delay and count of retransmissions for particular nrf24l01+ device.
		If retransmit count = 0 no retransmissions are produced.

	@param [in] nrf24_instance - pointer to the nrf24l01p instance for which function is called
	@param [in] new_retransmit_delay - new value of retransmission delay, should be expressed as nrf24_auto_retransmit_delay enum member
	@param [in] new_retransmit_count - new retransmit count

	@return first found mistake code or 0 if no mistakes were found
 */
// ************************************************ //
uint32_t nrf24_update_retransmission_params(nrf24l01p * nrf24_instance, nrf24_auto_retransmit_delay new_retransmit_delay, uint32_t new_retransmit_count)
{
	// Check if device was initialized
	if ( nrf24_instance->device_was_initialized == 0 )
	{
		return NRF24_INSTANCE_WAS_NOT_INITIALIZED;
	}

	uint32_t mistake_code = 0;

	if(new_retransmit_count < 0)
	{
		new_retransmit_count = 0;
		mistake_code = NRF24_WRONG_RETRANSMIT_COUNT;
	}
	else if ( new_retransmit_count > 15 )
	{
		new_retransmit_count = 15;
		mistake_code = NRF24_WRONG_RETRANSMIT_COUNT;
	}

	nrf24_instance->csn_low();
	nrf24_instance->spi_write_byte(NRF24_W_REGISTER | NRF24_SETUP_RETR);
	nrf24_instance->spi_write_byte(new_retransmit_delay | new_retransmit_count);
	nrf24_instance->csn_high();

	return mistake_code;
}

// EOF

