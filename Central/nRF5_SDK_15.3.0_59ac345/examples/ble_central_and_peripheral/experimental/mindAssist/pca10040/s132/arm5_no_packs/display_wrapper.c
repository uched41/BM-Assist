#include "display_wrapper.h"
#include "cfaf.h"
#include "nrf_log.h"
#include "config.h"

// Positions of each line
const uint8_t y_positions[5] = {45, 65, 85, 105, 125};

cfaf_t my_display = {
	._cs = DISPLAY_SS_PIN,
	._mosi = DISPLAY_MOSI_PIN,
	._miso = DISPLAY_MISO_PIN,
	._sck = DISPLAY_SCK_PIN,
	._reset = DISPLAY_RESET_PIN,
	._rs = DISPLAY_RS_PIN,
	.width = 128,
	.height = 128,
	.cursor_x = 3,
	.cursor_y = 5,
	.text_color = {0, 0, 0},
	.text_size = 1,
	.text_bgcolor = {0xff, 0xff, 0xff},
	.wrap = true,
};

void display_wrapper(void){
	Initialize_Cfaf(&my_display);
	
}

void write_reminders(cfaf_t* cfaf, user_data_t* user){
  //Fill display with a given RGB value
	On_Display(cfaf);
  Fill_LCD(cfaf, 0x87,0xce,0xeb);
  
	// Header part
	Set_Cursor(cfaf, (128 / 2) -  (sizeof("[ Mind Assist ]")*6/2) , 5);
	Write_Text(cfaf, "[ Mind Assist ]", sizeof("[ Mind Assist ]"));
	Set_Cursor(cfaf, 3, 25);
	uint8_t count = 0;
	
	for(uint8_t i=0; i<4; i++){
		if(user->reminder[i].is_set){
			Write_Text(cfaf, user->reminder[i].data, user->reminder[i].len);
			Set_Cursor(cfaf, 3, y_positions[count]);
			count++;
		}
	}
} 

void off_display(cfaf_t* cfaf){
	Fill_LCD(cfaf, 0x00,0x00,0x00);
	Off_Display(cfaf);
}