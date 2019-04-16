#include "utils.h"
#include "nrf_log.h"
#include "response.h"
#include "nrf_drv_systick.h"

const nrf_drv_rtc_t rtc = NRFX_RTC_INSTANCE(2); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

status_t status = {false, false};		// status object

/** @brief Function starting the internal LFCLK XTAL oscillator.
 */

static void rtc_handler(nrf_drv_rtc_int_type_t int_type){}
	

/** @brief Function initialization and configuration of RTC driver instance.
 */
static void rtc_config(void)
{
    uint32_t err_code;
    //Initialize RTC instance
    nrfx_rtc_config_t config = NRFX_RTC_DEFAULT_CONFIG;
    config.prescaler = 4095;
    err_code = nrfx_rtc_init(&rtc, &config, rtc_handler);
    APP_ERROR_CHECK(err_code);

    //Power on RTC instance
    nrf_drv_rtc_enable(&rtc);
}

void my_init(){
	NRF_LOG_INFO("Custom initialization.");
	nrfx_systick_init();
	rtc_config();
	tag_init();
}

