#include "buzzer.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "display_wrapper.h"
#include "utils.h"

/*
	Buzzer initialization
*/
void buzzer_init(buzzer_t* buzzer){
	nrf_gpio_cfg_output(buzzer->pin);
}

void buzzer_on(buzzer_t* buzzer){
	nrf_gpio_pin_set(buzzer->pin);
}

void buzzer_off(buzzer_t* buzzer){
	nrf_gpio_pin_clear(buzzer->pin);
}



/*
	Control Button
*/

void control_button_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
   NRF_LOG_INFO("Control button pressed.\n");
	 if(status.is_displaying){
		 buzzer_off(buzzer);
		 off_display(&my_display);
		 status.is_displaying = true;
	 }
}

void control_button_init(control_button_t* control_button){
	uint32_t err_code;
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	
	err_code = nrf_drv_gpiote_in_init(control_button->pin, &in_config, control_button_cb);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(control_button->pin, true);
}

