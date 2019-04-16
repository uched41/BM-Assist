#include <stdint.h>
#include <stdbool.h>
#include "ble.h"
#include "ble_srv_common.h"

#define CUSTOM_SERVICE_UUID_BASE         {0xDE, 0x8A, 0xBF, 0x45, 0xFA, 0x05, 0x50, 0xBA, \
                                          0x40, 0x42, 0xB0, 0xD0, 0xC9, 0xAD, 0x64, 0xF3}

#define CUSTOM_SERVICE_UUID               0x1400
#define CUSTOM_VALUE_CHAR_UUID            0x1401
												
#define MESSAGE_LENGTH 										20
#define CHARACTERISTIC_SIZE 							sizeof(ble_gap_addr_t) + MESSAGE_LENGTH
	
											
/**@brief   Macro for defining a ble_cus instance.
 *
 * @param   _name   Name of the instance.
 * @hideinitializer
 */
#define BLE_CUS_DEF(_name)                                                                          \
static ble_cus_t _name;                                                                             \
NRF_SDH_BLE_OBSERVER(_name ## _obs,                                                                 \
                     BLE_HRS_BLE_OBSERVER_PRIO,                                                     \
                     ble_cus_on_ble_evt, &_name)
																					

/**@brief Custom Service init structure. This contains all options and data needed for
 *        initialization of the service.*/
typedef struct
{
    uint8_t                       initial_custom_value;           /**< Initial custom value */
    ble_srv_cccd_security_mode_t  custom_value_char_attr_md;     /**< Initial security level for Custom characteristics attribute */
} ble_cus_init_t;


/**@brief Custom Service structure. This contains various status information for the service. */
struct ble_cus_s
{
    uint16_t                      service_handle;                 /**< Handle of Custom Service (as provided by the BLE stack). */
    ble_gatts_char_handles_t      custom_value_handles;           /**< Handles related to the Custom Value characteristic. */
    uint16_t                      conn_handle;                    /**< Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection). */
    uint8_t                       uuid_type; 
};


// Forward declaration of the ble_cus_t type.
typedef struct ble_cus_s ble_cus_t;

uint32_t ble_cus_init(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);
static uint32_t custom_value_char_add(ble_cus_t * p_cus, const ble_cus_init_t * p_cus_init);
void ble_cus_on_ble_evt( ble_evt_t const * p_ble_evt, void * p_context);
uint32_t ble_cus_custom_value_update(ble_cus_t * p_cus, uint8_t* custom_value, uint8_t len);
static void on_connect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt);
static void on_disconnect(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt);
static void on_write(ble_cus_t * p_cus, ble_evt_t const * p_ble_evt);


