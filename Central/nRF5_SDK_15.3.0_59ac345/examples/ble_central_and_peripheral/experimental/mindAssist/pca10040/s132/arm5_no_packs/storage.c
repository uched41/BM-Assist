#include "storage.h"
#include "nrf_log.h"
#include <stdlib.h>
#include "ble_cus.h"

#define REMINDERS_FILE	(0xF290)

static user_data_t user_data = {0};
static fds_record_t user_record = {0};

/* -------------------------
	bits 0 - 15 of user_id => record key
	bits 16 - 32 of user_id => file id
*/

/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;

/* Array to map FDS return values to strings. */
char const * fds_err_str[] =
{
    "FDS_SUCCESS",
    "FDS_ERR_OPERATION_TIMEOUT",
    "FDS_ERR_NOT_INITIALIZED",
    "FDS_ERR_UNALIGNED_ADDR",
    "FDS_ERR_INVALID_ARG",
    "FDS_ERR_NULL_ARG",
    "FDS_ERR_NO_OPEN_RECORDS",
    "FDS_ERR_NO_SPACE_IN_FLASH",
    "FDS_ERR_NO_SPACE_IN_QUEUES",
    "FDS_ERR_RECORD_TOO_LARGE",
    "FDS_ERR_NOT_FOUND",
    "FDS_ERR_NO_PAGES",
    "FDS_ERR_USER_LIMIT_REACHED",
    "FDS_ERR_CRC_CHECK_FAILED",
    "FDS_ERR_BUSY",
    "FDS_ERR_INTERNAL",
};

/* Array to map FDS events to strings. */
static char const * fds_evt_str[] =
{
    "FDS_EVT_INIT",
    "FDS_EVT_WRITE",
    "FDS_EVT_UPDATE",
    "FDS_EVT_DEL_RECORD",
    "FDS_EVT_DEL_FILE",
    "FDS_EVT_GC",
};




uint32_t pointer_to_int(uint8_t* addr, uint8_t len){
	return (addr[0]<<24) | (addr[1]<<16) | (addr[2]<<8) | (addr[3]);
}

void set_addr(addr_ble_t* a, const ble_gap_addr_t* b){
	memcpy(a->addr, b->addr, 6);
}

/*
	CRC functions
*/
uint32_t crc32_for_byte(uint32_t r) {
  for(int j = 0; j < 8; ++j)
    r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
  return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
  static uint32_t table[0x100];
  if(!*table)
    for(size_t i = 0; i < 0x100; ++i)
      table[i] = crc32_for_byte(i);
  for(size_t i = 0; i < n_bytes; ++i)
    *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}

/* 
	Generate unique ID to be used as file name 
*/
user_id_t generate_userid(const addr_ble_t* adr){
	uint32_t crc = 0;
	NRF_LOG_INFO("Generating ID for ");
	NRF_LOG_HEXDUMP_INFO(adr->addr, 6);
	crc32(adr->addr, 6, &crc);
	return crc;
}


/*
	Function to create user record
*/
user_id_t create_user(addr_ble_t* tag_addr){
	user_id_t user_id = generate_userid(tag_addr);
	NRF_LOG_INFO("Creating new user: 0x%08X ", user_id)
	
	memset(&user_data, 0, sizeof(user_data_t));
	
	fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
	
	uint16_t f_id = (uint16_t)((user_id & 0xffff0000) >> 16);
	uint16_t r_key = (uint16_t)(user_id & 0x0000ffff);
	
	ret_code_t rc = fds_record_find(f_id, r_key, &desc, &tok);
	if (rc == FDS_SUCCESS){		// first check if user already exists
		NRF_LOG_INFO("User Already exists, Aborting");
		return 0;
	}
		
	user_record.file_id = f_id;
	user_record.key =  r_key;
	user_record.data.p_data = &user_data;
	
	/* The length of a record is always expressed in 4-byte units (words). */
	user_record.data.length_words = (sizeof(user_data_t) + 3) / sizeof(uint32_t);
	
  rc = fds_record_write(&desc, &user_record);
	APP_ERROR_CHECK(rc);
	
	NRF_LOG_INFO("New user created, file_id: 0x%04X   record_key: 0x%04X\n", f_id, r_key);
	return user_id;
}


/*
	Function to update user data
*/	
uint32_t set_reminder(user_id_t user_id, uint8_t rem_no, uint8_t* data, uint8_t len){
	NRF_LOG_INFO("Updating user 0x%08X ", user_id);
	
	if(rem_no > 3 || len > MESSAGE_LENGTH){
		NRF_LOG_INFO("Incorrect params");
		return 1;
	}
	fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
		
  uint16_t f_id = (uint16_t)((user_id & 0xffff0000) >> 16);
	uint16_t r_key = (uint16_t)(user_id & 0x0000ffff);
	NRF_LOG_INFO("Setting reminder, user_id: 0x%08X   rem_no: 0x%02X", user_id, rem_no);
	
	ret_code_t rc = fds_record_find(f_id, r_key, &desc, &tok);
	if (rc == FDS_SUCCESS){
		NRF_LOG_INFO("User record found.\n");
		fds_flash_record_t record = {0};
		
		rc = fds_record_open(&desc, &record);
    APP_ERROR_CHECK(rc);
		
		memcpy(&user_data, record.p_data, sizeof(user_data_t));
		
		NRF_LOG_INFO("Updating user 0x%08X, reminder %d\n", user_id, rem_no);
		memcpy(user_data.reminder[rem_no].data, data, len);
		user_data.reminder[rem_no].len = len;
		user_data.reminder[rem_no].is_set = 1;
	
		user_record.file_id = f_id;
		user_record.key = r_key;
		user_record.data.p_data = &user_data;
		user_record.data.length_words = (sizeof(user_data_t) + 3) / sizeof(uint32_t);
				
		/* Close the record when done reading. */
		rc = fds_record_close(&desc);
		APP_ERROR_CHECK(rc);
		
		/* Write the updated record to flash. */
		rc = fds_record_update(&desc, &user_record);
		APP_ERROR_CHECK(rc);
	
		return 0;
	}
	else{
		NRF_LOG_INFO("User record NOT found.\n");
		return 1;
	}
}


/*
	Get the users data
*/
bool get_user_data(user_id_t user_id, user_data_t* ret_data){
	NRF_LOG_INFO("Getting user data 0x%08X", user_id);
	
	fds_record_desc_t desc = {0};
  fds_find_token_t  tok  = {0};
		
	uint16_t f_id = (uint16_t)((user_id & 0xffff0000) >> 16);
	uint16_t r_key = (uint16_t)(user_id & 0x0000ffff);
	
	NRF_LOG_INFO("Getting data, file_id: 0x%04X   record_key: 0x%04X", f_id, r_key);
	
	ret_code_t rc = fds_record_find(f_id, r_key, &desc, &tok);
	if (rc == FDS_SUCCESS){
		NRF_LOG_INFO("User record found.\n");
		fds_flash_record_t record = {0};
		
		rc = fds_record_open(&desc, &record);
    APP_ERROR_CHECK(rc);
		
		memcpy(ret_data, record.p_data, sizeof(user_data_t));
		
		/* Close the record when done reading. */
		rc = fds_record_close(&desc);
		APP_ERROR_CHECK(rc);
		return true;
	}
	else{
		NRF_LOG_INFO("User record NOT found.");
		return false;
	}
}


/*
	Function to handle initialization of FDS
*/
void storage_init(void){
		/* Register first to receive an event when initialization is complete. */
		NRF_LOG_INFO("Initializing FDS")
		(void) fds_register(fds_evt_handler);
		ret_code_t rc = fds_init();
		
		APP_ERROR_CHECK(rc);
		/* Wait for fds to initialize. */
		wait_for_fds_ready();
		
		fds_stat_t stat = {0};
		rc = fds_stat(&stat);
		APP_ERROR_CHECK(rc);

		NRF_LOG_INFO("Found %d valid records.", stat.valid_records);
		NRF_LOG_INFO("Found %d dirty records (ready to be garbage collected).", stat.dirty_records);
		
}


/**@brief   Wait for fds to initialize. */
static void wait_for_fds_ready(void)
{
    while (!m_fds_initialized){
       idle_state_handle();
    }
}


static void fds_evt_handler(fds_evt_t const * p_evt)
{
    NRF_LOG_INFO("Event: %s received (%s)",
                  fds_evt_str[p_evt->id],
                  fds_err_str[p_evt->result]);

    switch (p_evt->id)
    {
        case FDS_EVT_INIT:
            if (p_evt->result == FDS_SUCCESS)
            {
                m_fds_initialized = true;
            }
            break;

        case FDS_EVT_WRITE:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->write.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->write.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->write.record_key);
            }
        } break;

        case FDS_EVT_DEL_RECORD:
        {
            if (p_evt->result == FDS_SUCCESS)
            {
                NRF_LOG_INFO("Record ID:\t0x%04x",  p_evt->del.record_id);
                NRF_LOG_INFO("File ID:\t0x%04x",    p_evt->del.file_id);
                NRF_LOG_INFO("Record key:\t0x%04x", p_evt->del.record_key);
            }
            
        } break;

        default:
            break;
    }
}






