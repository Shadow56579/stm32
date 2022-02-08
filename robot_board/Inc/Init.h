#ifndef INIT_H_
#define INIT_H_

#include <stdint.h>
#include "nrf24l01p_impi.h"

void device_init(nrf24l01p* nrf24l01p_instance);
void drv_init();
void led_init();
void timer1_init();
void timer2_init();
void spi1_init();

#endif /* INIT_H_ */
