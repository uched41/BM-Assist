#include "sdk_common.h"
#include "ble_srv_common.h"
#include "ble_cus.h"
#include <string.h>
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_log.h"  
#include "storage.h"

/*
	Custom service initializer 
	*/
uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    if (p_cus == NULL || p_cus_init == NULL)
    {
        return NRF_ERROR_NULL;
    }

    uint32_t   err_code;
    ble_uuid_t ble_uuid;
		
		// Initialize service structure
		p_cus->conn_handle               = BLE_CONN_HANDLE_INVALID;
		
		// Add Custom Service UUID
		ble_uuid128_t base_uuid = {CUSTOM_SERVICE_UUID_BASE};
		err_code =  sd_ble_uuid_vs_add(&base_uuid, &p_cus->uuid_type);
		VERIFY_SUCCESS(err_code);

		ble_uuid.type = p_cus->uuid_type;
		ble_uuid.uuid = CUSTOM_SERVICE_UUID;
		
		// Add the Custom Service
		err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid, &p_cus->service_handle);
		if (err_code != NRF_SUCCESS){
				return err_code;
		}
		
		// Add Custom Value characteristic
    return custom_value_char_add(p_cus, p_cus_init);
}


/*
	Custom characteristic initializer
*/
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init)
{
    uint32_t            err_code;
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;
	
		memset(&cccd_md, 0, sizeof(cccd_md));

    //  Read  operation on Cccd should be possible without authentication.
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    
    cccd_md.vloc       = BLE_GATTS_VLOC_STACK;
	
		memset(&char_md, 0, sizeof(char_md));
		char_md.char_props.read   = 1;
    char_md.char_props.write  = 1;
    char_md.char_props.notify = 1;
		char_md.p_cccd_md         = &cccd_md; 
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = NULL; 
    char_md.p_sccd_md         = NULL;
	
		memset(&attr_md, 0, sizeof(attr_md));
    attr_md.read_perm  = p_cus_init->custom_value_char_attr_md.read_perm;
    attr_md.write_perm = p_cus_init->custom_value_char_attr_md.write_perm;
    attr_md.vloc       = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth    = 0;
    attr_md.wr_auth    = 0;
    attr_md.vlen       = 0;
		
		ble_uuid.type = p_cus->uuid_type;
    ble_uuid.uuid = CUSTOM_VALUE_CHAR_UUID;
		
		memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = CHARACTERISTIC_SIZE;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = CHARACTERISTIC_SIZE;
		
		err_code = sd_ble_gatts_characteristic_add(p_cus->service_handle, &char_md,
							 &attr_char_value,
							 &p_cus->custom_value_handles);
    if (err_code != NRF_SUCCESS){
        return err_code;
    }
    return NRF_SUCCESS;
}


/*
	Function to update value, i.e when reply to sender
*/
uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t* custom_value, uint8_t len){
    if (p_cus == NULL){
        return NRF_ERROR_NULL;
    }
		
		uint32_t err_code = NRF_SUCCESS;
		ble_gatts_value_t gatts_value;
		
		// Initialize value struct.
		memset(&gatts_value, 0, sizeof(gatts_value));

		gatts_value.len     = len;
		gatts_value.offset  = 0;
		gatts_value.p_value = custom_value;

		// Update database.
		err_code = sd_ble_gatts_value_set(p_cus->conn_handle,
																				p_cus->custom_value_handles.value_handle,
																				&gatts_value);
		if (err_code != NRF_SUCCESS){
				return err_code;
		}
		// Send value if connected and notifying.
		if ((p_cus->conn_handle != BLE_CONN_HANDLE_INVALID)) {
				ble_gatts_hvx_params_t hvx_params;
				memset(&hvx_params, 0, sizeof(hvx_params));

				hvx_params.handle = p_cus->custom_value_handles.value_handle;
				hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
				hvx_params.offset = gatts_value.offset;
				hvx_params.p_len  = &gatts_value.len;
				hvx_params.p_data = gatts_value.p_value;

				err_code = sd_ble_gatts_hvx(p_cus->conn_handle, &hvx_params);
		}
		else{
				err_code = NRF_ERROR_INVALID_STATE;
		}

		return err_code;
}



/*
	Custom service event handler
*/
void ble_cus_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_cus_t * p_cus = (ble_cus_t *) p_context;
    
    if (p_cus == NULL || p_ble_evt == NULL){
        return;
    }
		
		switch (p_ble_evt->header.evt_id)
		{
			case BLE_GAP_EVT_CONNECTED:
				on_connect(p_cus, p_ble_evt);
				break;

			case BLE_GAP_EVT_DISCONNECTED:
				on_disconnect(p_cus, p_ble_evt);
				break;

			case BLE_GATTS_EVT_WRITE:
				on_write(p_cus,  p_ble_evt);
        break;
			default:
					// No implementation needed.
					break;
    }
}


/*
	Handle connection 
*/
static void on_connect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt){
    p_cus->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
}


/*
	Handle disconnection 
*/
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt){
    UNUSED_PARAMETER(p_ble_evt);
    p_cus->conn_handle = BLE_CONN_HANDLE_INVALID;
}

/*
	Handle write from User
*/
static void on_write(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt){
		ble_gatts_evt_write_t * p_evt_write = (ble_gatts_evt_write_t *)&p_ble_evt->evt.gatts_evt.params.write;
    
    // Check if the handle passed with the event matches the Custom Value Characteristic handle.
    if (p_evt_write->handle == p_cus->custom_value_handles.value_handle)
    {
        // Put specific task here. 
			NRF_LOG_INFO("Data received: %d\n Parsing data: \n", p_evt_write->len);
			NRF_LOG_HEXDUMP_INFO(p_evt_write->data, p_evt_write->len);
			
			uint8_t cmd = p_evt_write->data[0];
			
			switch(cmd){
				case 1:{			// Create new user
					addr_ble_t new_addr;
					memcpy(new_addr.addr, p_evt_write->data+1, 6);
					NRF_LOG_HEXDUMP_INFO(new_addr.addr, 6);
					user_id_t u_id = create_user(&new_addr);
					
					ret_code_t err_code = ble_cus_custom_value_update(p_cus, (uint8_t*)&u_id, sizeof(uint32_t));
					APP_ERROR_CHECK(err_code);
					break;
				}
				
				case 2:{			// Set reminder
					uint8_t len = p_evt_write->data[6];
					uint8_t rem_no = p_evt_write->data[5];
					uint32_t u_id = pointer_to_int(p_evt_write->data+1, 4); 
					
					uint32_t ans = set_reminder(u_id, rem_no, p_evt_write->data+7, len);
					ret_code_t err_code = ble_cus_custom_value_update(p_cus, (uint8_t*)&ans, sizeof(uint32_t));
					APP_ERROR_CHECK(err_code);
					break;
				}
				default:
					break;
			}
		}
}


