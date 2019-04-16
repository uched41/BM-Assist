#include "response.h"
#include <stdlib.h>
#include "nrf_log.h"
#include "config.h"

Tag* default_tag;		// pointer to default tag

void tag_init(void){
	default_tag = malloc(sizeof(Tag));
	default_tag->rssi = 0;
	default_tag->after = NULL;
	default_tag->before = NULL;
}

void add_tag(Tag* tag, addr_ble_t paddr, int8_t rssi){
	NRF_LOG_INFO("Adding new tag: ");
	NRF_LOG_HEXDUMP_INFO(&paddr, sizeof(paddr));
	
	tag = malloc(sizeof(Tag));
	tag->p_addr = paddr;
	tag->rssi = rssi;
	tag->after = NULL;
	tag->before = last_tag();
	tag->last_time = nrfx_rtc_counter_get(&rtc);
	
	((Tag*)(tag->before))->after = tag;		// set predecessor
}

void remove_tag(Tag* tag){
	NRF_LOG_INFO("Removing tag: \n");
	NRF_LOG_HEXDUMP_INFO(&tag->p_addr, sizeof(tag->p_addr));
	
	((Tag*)(tag->before))->after = tag->after;
	((Tag*)(tag->after))->before = tag->before;
	
	free(tag);
}

Tag* last_tag(void){
	Tag* ttag = default_tag;
	
	while(ttag->after != NULL){
		ttag = ttag->after;
	}
	return ttag;
}

bool address_equal(addr_ble_t addr1, addr_ble_t addr2){
		for(uint8_t i=0; i<sizeof(addr1.addr); i++){
			if(addr1.addr[i] != addr2.addr[i]){
				return false;
			}
		}
		return true;
}

bool is_tag(addr_ble_t paddr){
	Tag* ttag = default_tag->after;
	
	while(ttag != NULL){
		if(address_equal(ttag->p_addr, paddr)){
			return true;
		}
		ttag = ttag->after;
	}
	return false;
}

Tag* get_tag(addr_ble_t paddr){
	Tag* ttag = default_tag->after;
	
	while(ttag != NULL){
		if(address_equal(ttag->p_addr, paddr)){
			return ttag;
		}
		ttag = ttag->after;
	}
	return NULL;
}

bool tag_present(Tag* tag){
	uint32_t current_time = nrfx_rtc_counter_get(&rtc);
	if(current_time - tag->last_time > PROXIMITY_TIMEOUT )
		return false;
	
	if(abs(tag->rssi) > 55)
		return false;
			
	return true;
}
	
void check_tags(void){
	NRF_LOG_INFO("Checking tags in proximity... \n");
	Tag* ttag = default_tag->after;
	
	while(ttag != NULL){
		if(!tag_present(ttag)){			// if tag is no longer present
			Tag* next_tag = ttag->after;
			remove_tag(ttag);
			ttag = next_tag;
		}
		else{
			ttag = ttag->after;
		}
	}
	NRF_LOG_INFO("Done");
}

void update_tag(addr_ble_t paddr, int8_t rssi){
	if(is_tag(paddr)){
		NRF_LOG_INFO("Updating tag: ");
		NRF_LOG_HEXDUMP_INFO(&paddr, 6);
		NRF_LOG_INFO("RSSI: %d \n", rssi);
		
		Tag* old_tag = get_tag(paddr);
		old_tag->rssi = rssi;
		
		old_tag->last_time = nrfx_rtc_counter_get(&rtc);	
	}
	else{
		Tag* new_tag;
		add_tag(new_tag, paddr, rssi);
	}
}


