#ifndef CONFIG_H
#define CONFIG_H

#define PROXIMITY_TIMEOUT	10 * 8  	// (10 secs) Time after which we assume that the tag is no longer in range
																	
												
/*
	Pin definitions
*/
#define ACCEL_INTERRUPT_PIN 	19
#define ACCEL_SCL_PIN 				26
#define ACCEL_SDA_PIN 				25

#define DISPLAY_MISO_PIN 			14
#define DISPLAY_MOSI_PIN 			13
#define DISPLAY_SCK_PIN 			12
#define DISPLAY_SS_PIN 				15
#define DISPLAY_RESET_PIN			23
#define DISPLAY_RS_PIN				24

#define BUZZER_PIN						7
#define CONTROL_BUTTON_PIN		6

#endif

