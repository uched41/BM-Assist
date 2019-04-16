#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

typedef struct{
	uint8_t pin;
}buzzer_t;

extern buzzer_t* buzzer;

typedef struct{
	uint8_t pin;
}control_button_t;

extern control_button_t* control_button;

void buzzer_init(buzzer_t* buzzer);
void buzzer_on(buzzer_t* buzzer);
void buzzer_off(buzzer_t* buzzer);

void control_button_init(control_button_t* control_button);
	
#endif
