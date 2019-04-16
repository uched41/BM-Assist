#ifndef RESPONSE_H
#define RESPONSE_H

#include "ble.h"
#include "nrf_ble_scan.h"
#include "nrf_drv_rtc.h"
#include "nrf_drv_clock.h"
#include "storage.h"

extern const nrf_drv_rtc_t rtc;

/*
	Tags are stored in a linked list and updatated.
	
	** Note:
	We may have function (check_tag) which regularly run checks to remove tags
	that have gone out of range, but we are currently not using this function.
	*/
typedef struct{
	addr_ble_t p_addr;		// peer address
	int8_t rssi;
	uint32_t last_time;		  	// Last time we received advertising data from tag
	void* before;							// pointer to the structure before this tag, in our linked list
	void* after;
}Tag;

extern Tag* default_tag;

void tag_init(void);
	
Tag* find_tag(addr_ble_t addr);	// Function that finds index of tag in our array

Tag* last_tag(void);

void add_tag(Tag* tag, addr_ble_t paddr, int8_t rssi);

void remove_tag(Tag* tag);		// Function that checks if the tags are still in proximity
															// removes tags which are no longer in proximity

bool tag_present(Tag* tag);

bool is_tag(addr_ble_t paddr);

bool address_equal(addr_ble_t addr1, addr_ble_t addr2);

Tag* get_tag(addr_ble_t paddr);

void update_tag(addr_ble_t paddr, int8_t rssi);

#endif
