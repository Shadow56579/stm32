#ifndef IMPLEMENTATION_H_
#define IMPLEMENTATION_H_

#include <stdint.h>
#include "stm32g071xx.h"

static void delay(uint32_t dl);

uint8_t spi1_write_function(uint8_t byte_to_write);
void ce_high_function(void);
void ce_low_function(void);
void csn_high_function(void);
void csn_low_function(void);
void encode_buttons(uint8_t radio_controller_message[6]);
void encode_joystick(uint8_t right_joystick_top_bottom, uint8_t right_joystick_left_right,
					 uint8_t left_joystick_left_right, uint8_t left_joystick_top_bottom,
					 uint8_t radio_controller_message[6]);
void reset_joystick(void);
void reset_buttons(void);




#endif /* IMPLEMENTATION_H_ */
