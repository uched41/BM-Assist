#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef struct{
	volatile bool is_displaying;
	volatile bool is_door_open;
}status_t;

extern status_t status;

void my_init(void);

#endif

