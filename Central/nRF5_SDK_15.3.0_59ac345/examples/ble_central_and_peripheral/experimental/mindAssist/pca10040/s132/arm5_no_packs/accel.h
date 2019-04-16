#ifndef ACCEL_H
#define ACCEL_H


#include "nrf_drv_gpiote.h"

#define MPU_INT_PIN      28

void accel_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

void accel_init(void);

#endif