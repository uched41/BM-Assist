#ifndef DISPLAY_WRAPPER_H
#define DISPLAY_WRAPPER_H

#include "cfaf.h"
#include "storage.h"

extern cfaf_t my_display;

void display_wrapper(void);

void write_reminders(cfaf_t* cfaf, user_data_t* user);

void off_display(cfaf_t* cfaf);

#endif 