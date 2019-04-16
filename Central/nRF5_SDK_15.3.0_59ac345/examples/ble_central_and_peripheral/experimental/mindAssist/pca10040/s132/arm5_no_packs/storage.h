#ifndef STORAGE_H
#define STORAGE_H

#include "fds.h"
#include "nrf_ble_scan.h"

typedef struct{
	uint8_t addr[6];
}addr_ble_t;

typedef struct{
	uint32_t boot_count;
	char     device_name[16];
	uint8_t  num_tags;
} configuration_t;

typedef struct{
	char data[20];
	uint8_t len;
	bool is_set;
}reminder_t;

typedef struct{
	reminder_t reminder[4];
}user_data_t;

typedef uint32_t user_id_t;

void set_addr(addr_ble_t* a, const ble_gap_addr_t* b);

// crc functions
uint32_t crc32_for_byte(uint32_t r);
void crc32(const void *data, size_t n_bytes, uint32_t* crc);

// fds functions
static void fds_evt_handler(fds_evt_t const * p_evt);
void storage_init(void);
void idle_state_handle(void);
static void wait_for_fds_ready(void);

// User functions
user_id_t generate_userid(const addr_ble_t* addr);
user_id_t create_user(addr_ble_t* tag_addr);
uint32_t set_reminder(user_id_t user, uint8_t rem_no, uint8_t* data, uint8_t len);
bool get_user_data(user_id_t user, user_data_t* user_data);

uint32_t pointer_to_int(uint8_t* addr, uint8_t len);

#endif

