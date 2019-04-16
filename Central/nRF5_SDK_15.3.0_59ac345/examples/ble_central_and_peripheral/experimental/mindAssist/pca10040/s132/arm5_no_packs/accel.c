#include "app_mpu.h"
#include "accel.h"

void accel_init(void){
	unsigned char accel_fsr,  new_temp = 0;
  unsigned short gyro_rate, gyro_fsr;
  unsigned long timestamp;

  // Set up interrupt pin
	ret_code_t err_code;
	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	//in_config.pull = NRF_GPIO_PIN_NOPULL;

	err_code = nrf_drv_gpiote_in_init(MPU_INT_PIN, &in_config, accel_cb);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(MPU_INT_PIN, true);
		
	// hardware initialization of mpu6050
	app_mpu_init();				
}

void accel_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    
}
