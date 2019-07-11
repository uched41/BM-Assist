#include "nrf_drv_gpiote.h"
#include "accel_wrapper.h"
#include "app_mpu.h"
#include "config.h"
#include "nrf_log.h"
#include "response.h"
#include "storage.h"
#include "display_wrapper.h"
#include "utils.h"
#include "buzzer.h"

/* Configuration for accelerometer */
static app_mpu_config_t accel_config = MPU_DEFAULT_CONFIG();
static app_mpu_int_pin_cfg_t accel_int_config = MPU_DEFAULT_INT_PIN_CONFIG();
static app_mpu_int_enable_t accel_int_enable = MPU_DEFAULT_INT_ENABLE_CONFIG();

static gyro_values_t gyro_vals;

/*
	Callback for accelerometer interrupt
*/
void accel_cb(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
   NRF_LOG_INFO("acceleromer triggered \n");
	 if(!status.is_displaying){
		 status.is_displaying = true;
		 remind(); 
	 }
}


/*
	Accelerometer initialization,
Note: to adjust motion detection sensitivity, adjust motion detection threshold
	and motion detection duration
*/
void accel_init(void){
	// Set up interrupt pin
	ret_code_t err_code;
	err_code = nrf_drv_gpiote_init();
	//APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	//in_config.pull = NRF_GPIO_PIN_NOPULL;

	err_code = nrf_drv_gpiote_in_init(ACCEL_INTERRUPT_PIN, &in_config, accel_cb);
	APP_ERROR_CHECK(err_code);

	nrf_drv_gpiote_in_event_enable(ACCEL_INTERRUPT_PIN, true);
		
	// hardware initialization of mpu6050
	err_code = app_mpu_init();		
	APP_ERROR_CHECK(err_code);
	
	err_code = app_mpu_config(&accel_config);		
	APP_ERROR_CHECK(err_code);
	
	err_code = app_mpu_int_cfg_pin(&accel_int_config);		
	APP_ERROR_CHECK(err_code);
	
	err_code = app_mpu_motion_detect_threshold(2);
	
	accel_int_enable.mot_en = 1;
	err_code = app_mpu_int_enable(&accel_int_enable);		
	APP_ERROR_CHECK(err_code);
	
	NRF_LOG_INFO("Accelerometer initialization complete.")
}


/*
	Function to check if door is open 
*/
bool is_door_open(){
	//app_mpu_read_gyro(&gyro_vals);
	//NRF_LOG_INFO("Gyro values x:%d, y:%d, z:%d \n", gyro_vals.x, gyro_vals.y, gyro_vals.z);
	return true;
}

void remind(void){
	// Iterate through all avialable tags
	NRF_LOG_INFO("Iterating through tags ...");
	Tag* ttag = default_tag->after;
	
	while(ttag != NULL){
		NRF_LOG_HEXDUMP_INFO(&ttag->p_addr.addr, 6);
		if(tag_present(ttag)){
			
			NRF_LOG_INFO("Tag present ! ");
			user_id_t u_id = generate_userid(&ttag->p_addr);
			NRF_LOG_INFO("User ID: 0x%08X", u_id);
			
			user_data_t user_data = {0};
			
			if ( get_user_data(u_id, &user_data) ){			
				buzzer_on(buzzer);	// Buzzer on			
				write_reminders(&my_display, &user_data);	// Show reminders on screen
				NRF_LOG_HEXDUMP_INFO(&user_data, sizeof(user_data_t));
			} else {
				NRF_LOG_INFO("User data not found.");
			}
			
		}
		else{
			NRF_LOG_INFO("Tag not in close proximity.");
		}
		ttag = ttag->after;	// move to next tag in linked list
		NRF_LOG_INFO("\n");
	}
	NRF_LOG_INFO("Done\n");
}

