#include "cfaf.h"
#include "nrf_log.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "font.c"
#include <stdlib.h>
#include "config.h"
#include "nrf_drv_spi.h"

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define swap(a, b) { int16_t t = a; a = b; b = t; }
#define equal_col(a, c) (a.r == c.r && c.g == a.g && a.b == c.b)

#define display CFAF240320X020T

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(1);  /**< SPI instance. */

void CLR_RS(cfaf_t* cfaf){
	nrf_gpio_pin_clear(cfaf->_rs);
}

void SET_RS(cfaf_t* cfaf){
	nrf_gpio_pin_set(cfaf->_rs);
}

void CLR_RESET(cfaf_t* cfaf){
	nrf_gpio_pin_clear(cfaf->_reset);
}

void SET_RESET(cfaf_t* cfaf){
	nrf_gpio_pin_set(cfaf->_reset);
}

void CLR_CS(cfaf_t* cfaf){
	nrf_gpio_pin_clear(cfaf->_cs);
}

void SET_CS(cfaf_t* cfaf){
	nrf_gpio_pin_set(cfaf->_cs);
}

void CLR_MOSI(cfaf_t* cfaf){
	nrf_gpio_pin_clear(cfaf->_mosi);
}

void SET_MOSI(cfaf_t* cfaf){
	nrf_gpio_pin_set(cfaf->_mosi);
}

void CLR_SCK(cfaf_t* cfaf){
	nrf_gpio_pin_clear(cfaf->_sck);
}

void SET_SCK(cfaf_t* cfaf){
	nrf_gpio_pin_set(cfaf->_sck);
}


bool Initialize_Cfaf(cfaf_t* cfaf){
	ret_code_t err_code;
	
	// initialize test pin
	nrf_gpio_cfg_output(cfaf->_reset);
	nrf_gpio_cfg_output(cfaf->_rs);
	nrf_gpio_cfg_output(DISPLAY_BACKLIGHT);
	//nrf_gpio_pin_set(DISPLAY_BACKLIGHT);
	
	//Drive the ports to a reasonable starting state.
  CLR_RESET(cfaf);
  CLR_RS(cfaf);
  SET_CS(cfaf);
																
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = cfaf->_cs;
	spi_config.mosi_pin = cfaf->_mosi;
	spi_config.miso_pin = cfaf->_miso;
	spi_config.sck_pin  = cfaf->_sck;
	spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
	
	err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
	NRF_LOG_INFO("SPI : 0x%04X", err_code);
	APP_ERROR_CHECK(err_code);
	NRF_LOG_INFO("SPI started.");
	
	Setup_Lcd(cfaf);
	
	NRF_LOG_INFO("Display initialization complete.");

  return true;
}

void Setup_Lcd(cfaf_t* cfaf){
	//Reset the LCD controller
  CLR_RESET(cfaf);
  delay(1);//10µS min
  SET_RESET(cfaf);
  delay(150);//120mS max

  SPI_sendCommand(cfaf, ST7789_11_SLPOUT);
  delay(120);

  // MADCTL (36h): Memory Data Access Control
  // Set the RGB vs BGR order to match a windows 24-bit BMP 
  SPI_sendCommand(cfaf, ST7789_36_MADCTL);
  SPI_sendData(cfaf, 0x08);// YXVL RH--

  // COLMOD (3Ah): Interface Pixel Format
  // * This command is used to define the format of RGB picture
  //   data, which is to be transferred via the MCU interface.
  SPI_sendCommand(cfaf, ST7789_3A_COLMOD);
  SPI_sendData(cfaf, 0x06);// Default: 0x06 => 18-bit/pixel
                     // IFPF[2:0] MCU Interface Color Format
                     // IFPF[2:0] | Format
                     //      000b | reserved
                     //      001b | reserved
                     //      010b | reserved
                     //      011b | 12-bit/pixel
                     //      100b | reserved
                     //      101b | 16-bit/pixel
                     //      110b | 18-bit/pixel   <<<<<
                     //      111b | reserved


  // PORCTRL (B2h): Porch Setting
  SPI_sendCommand(cfaf, ST7789_B2_PORCTRL);
  SPI_sendData(cfaf, 0x0C);   //BPA[6:0]: Back porch setting in normal mode. The minimum setting is 0x01.
  SPI_sendData(cfaf, 0x0C);   //FPA[6:0]: Front porch setting in normal mode. The minimum setting is 0x01.
  SPI_sendData(cfaf, 0x00);   //Disable(0) / Enable (1) separate porch control
  SPI_sendData(cfaf, 0x33);   //(high nibble) BPB[3:0]: Back porch setting in idle mode. The minimum setting is 0x01.
                        //(low nibble)  FPB[3:0]: Front porch setting in idle mode. The minimum setting is 0x01.
  SPI_sendData(cfaf, 0x33);   //(high nibble) BPB[3:0]: Back porch setting in partial mode. The minimum setting is 0x01.
                        //(low nibble)  FPC[3:0]: Front porch setting in partial mode. The minimum setting is 0x01.

  // GCTRL (B7h): Gate Control
  SPI_sendCommand(cfaf, ST7789_B7_GCTRL);
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(cfaf, 0x34);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(cfaf, 0x35);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(cfaf, 0x70);
#endif

  // VCOMS (BBh): VCOM Setting
  SPI_sendCommand(cfaf, ST7789_BB_VCOMS);
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(cfaf, 0x1C);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(cfaf, 0x2B);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(cfaf, 0x35);
#endif

  // LCMCTRL (C0h): LCM Control
  SPI_sendCommand(cfaf, ST7789_C0_LCMCTRL);     
  SPI_sendData(cfaf, 0x2C);   
   
  // VDVVRHEN (C2h): VDV and VRH Command Enable
  SPI_sendCommand(cfaf, ST7789_C2_VDVVRHEN);     
  SPI_sendData(cfaf, 0x01); // CMDEN=”0”: VDV and VRH register value comes from NVM.
                      // CMDEN=”1”, VDV and VRH register value comes from command write.
  SPI_sendData(cfaf, 0xFF);   

  // VRHS (C3h): VRH Set
  SPI_sendCommand(cfaf, ST7789_C3_VRHS);
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(cfaf, 0x0B);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(cfaf, 0x20);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(cfaf, 0x11);
#endif

  SPI_sendCommand(cfaf, ST7789_C4_VDVSET);
  SPI_sendData(cfaf, 0x20);
  
  //FRCTRL2 (C6h): Frame Rate Control in Normal Mode
  SPI_sendCommand(cfaf, ST7789_C6_FRCTR2);
  SPI_sendData(cfaf, 0x0F);
  
  // PWCTRL1 (D0h): Power Control 1
  SPI_sendCommand(cfaf, ST7789_D0_PWCTRL1);
  SPI_sendData(cfaf, 0xA4);   //Fixed vector
  SPI_sendData(cfaf, 0xA1);   //AVDD=6.8V, AVCL=-4.8V, VDS=2.3V
    
  // PVGAMCTRL (E0h): Positive Voltage Gamma Control
  SPI_sendCommand(cfaf, ST7789_E0_PVGAMCTRL);     
  // Pile of magic numbers :-(
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(cfaf, 0xD0);   
  SPI_sendData(cfaf, 0x00);   
  SPI_sendData(cfaf, 0x03);   
  SPI_sendData(cfaf, 0x09);   
  SPI_sendData(cfaf, 0x09);   
  SPI_sendData(cfaf, 0x17);   
  SPI_sendData(cfaf, 0x2A);   
  SPI_sendData(cfaf, 0x44);   
  SPI_sendData(cfaf, 0x3C);   
  SPI_sendData(cfaf, 0x2B);   
  SPI_sendData(cfaf, 0x17);   
  SPI_sendData(cfaf, 0x15);   
  SPI_sendData(cfaf, 0x10);   
  SPI_sendData(cfaf, 0x13);  
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(cfaf, 0xD0);
  SPI_sendData(cfaf, 0xCA);
  SPI_sendData(cfaf, 0x0E);
  SPI_sendData(cfaf, 0x08);
  SPI_sendData(cfaf, 0x09);
  SPI_sendData(cfaf, 0x07);
  SPI_sendData(cfaf, 0x2D);
  SPI_sendData(cfaf, 0x3B);
  SPI_sendData(cfaf, 0x3D);
  SPI_sendData(cfaf, 0x34);
  SPI_sendData(cfaf, 0x0A);
  SPI_sendData(cfaf, 0x0A);
  SPI_sendData(cfaf, 0x1B);
  SPI_sendData(cfaf, 0x28);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(cfaf, 0xd0);
  SPI_sendData(cfaf, 0x00);
  SPI_sendData(cfaf, 0x06);
  SPI_sendData(cfaf, 0x09);
  SPI_sendData(cfaf, 0x0b);
  SPI_sendData(cfaf, 0x2a);
  SPI_sendData(cfaf, 0x3c);
  SPI_sendData(cfaf, 0x55);
  SPI_sendData(cfaf, 0x4b);
  SPI_sendData(cfaf, 0x08);
  SPI_sendData(cfaf, 0x16);
  SPI_sendData(cfaf, 0x14);
  SPI_sendData(cfaf, 0x19);
  SPI_sendData(cfaf, 0x20);
#endif

  // NVGAMCTRL (E1h): Negative Voltage Gamma Control
  SPI_sendCommand(cfaf, ST7789_E1_NVGAMCTRL);     
  // Pile of magic numbers :-(
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(cfaf, 0xD0);   
  SPI_sendData(cfaf, 0x00);   
  SPI_sendData(cfaf, 0x02);   
  SPI_sendData(cfaf, 0x08);   
  SPI_sendData(cfaf, 0x08);   
  SPI_sendData(cfaf, 0x27);   
  SPI_sendData(cfaf, 0x26);   
  SPI_sendData(cfaf, 0x54);   
  SPI_sendData(cfaf, 0x3B);   
  SPI_sendData(cfaf, 0x3B);   
  SPI_sendData(cfaf, 0x16);   
  SPI_sendData(cfaf, 0x15);   
  SPI_sendData(cfaf, 0x0F);   
  SPI_sendData(cfaf, 0x13); 
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(cfaf, 0xD0);
  SPI_sendData(cfaf, 0xCA);
  SPI_sendData(cfaf, 0x0F);
  SPI_sendData(cfaf, 0x08);
  SPI_sendData(cfaf, 0x08);
  SPI_sendData(cfaf, 0x07);
  SPI_sendData(cfaf, 0x2E);
  SPI_sendData(cfaf, 0x5C);
  SPI_sendData(cfaf, 0x40);
  SPI_sendData(cfaf, 0x34);
  SPI_sendData(cfaf, 0x09);
  SPI_sendData(cfaf, 0x0B);
  SPI_sendData(cfaf, 0x1B);
  SPI_sendData(cfaf, 0x28);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(cfaf, 0xd0);
  SPI_sendData(cfaf, 0x00);
  SPI_sendData(cfaf, 0x06);
  SPI_sendData(cfaf, 0x09);
  SPI_sendData(cfaf, 0x0b);
  SPI_sendData(cfaf, 0x29);
  SPI_sendData(cfaf, 0x36);
  SPI_sendData(cfaf, 0x54);
  SPI_sendData(cfaf, 0x4b);
  SPI_sendData(cfaf, 0x0d);
  SPI_sendData(cfaf, 0x16);
  SPI_sendData(cfaf, 0x14);
  SPI_sendData(cfaf, 0x21);
  SPI_sendData(cfaf, 0x20);
#endif
 

#if(display == CFAF240320V020T)
  // Normal TN TFT
  // INVOFF (20h): Display Inversion Off (correct for V)
  SPI_sendCommand(cfaf, ST7789_20_INVOFF);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  // INVON (21h): Display Inversion On (correct for W)
  SPI_sendCommand(cfaf, ST7789_21_INVON); 
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  // INVOFF (20h): Display Inversion Off (correct for X)
  SPI_sendCommand(cfaf, ST7789_20_INVOFF);
#endif

  // CASET (2Ah): Column Address Set
  SPI_sendCommand(cfaf, ST7789_2A_CASET);
  SPI_sendData(cfaf, 0x00); //Start MSB Start = 0
  SPI_sendData(cfaf, 0x00); //Start LSB
  SPI_sendData(cfaf, 0x00); //End MSB End = 249
  SPI_sendData(cfaf, 0xEF); //End LSB

  // RASET (2Bh): Row Address Set
  SPI_sendCommand(cfaf, ST7789_2B_RASET);
  SPI_sendData(cfaf, 0x00); //Start MSB Start = 0
  SPI_sendData(cfaf, 0x00); //Start LSB
  SPI_sendData(cfaf, 0x01); //End MSB End = 319
  SPI_sendData(cfaf, 0x3F); //End LSB

  // DISPON (29h): Display On
  SPI_sendCommand(cfaf, ST7789_29_DISPON);
  delay(1);
  
	//delay(100);
	
	//Off_Display(cfaf);
}

void Off_Display(cfaf_t* cfaf){
	SPI_sendCommand(cfaf, ST7789_28_DISPOFF);//Display Off
}

void On_Display(cfaf_t* cfaf){
	SPI_sendCommand(cfaf, ST7789_29_DISPON);//Display On
}

void SPI_sendCommand(cfaf_t* cfaf, uint8_t command){
  CLR_RS(cfaf);
  CLR_CS(cfaf);
	
  //Send the command via SPI:
	uint8_t cmd = command;
  //APP_ERROR_CHECK(
		nrf_drv_spi_transfer(&spi, &cmd, 1, NULL, 0);
	//);
  SET_CS(cfaf);
 }


void SPI_sendData(cfaf_t* cfaf, uint8_t data){
  SET_RS(cfaf);
  CLR_CS(cfaf);
	
  //Send the command via SPI:
	uint8_t dt = data;
  //APP_ERROR_CHECK(
		nrf_drv_spi_transfer(&spi, &dt, 1, NULL, 0);
	//);
  SET_CS(cfaf);
}

void Set_LCD_for_write_at_X_Y(cfaf_t* cfaf, int x, int y){	
  SPI_sendCommand(cfaf, ST7789_2A_CASET); //Column address set
  //Write the parameters for the "column address set" command
  SPI_sendData(cfaf, x>>8);     //Start MSB = XS[15:8]
  SPI_sendData(cfaf, x&0x00FF); //Start LSB = XS[ 7:0]
  SPI_sendData(cfaf, 0);        //End MSB   = XE[15:8] 240-1
  SPI_sendData(cfaf, 240);      //End LSB   = XE[ 7:0]
  
  SPI_sendCommand(cfaf, ST7789_2B_RASET); //Row address set
 
  y=319-y;
  SPI_sendData(cfaf, y>>8);     //Start MSB = YS[15:8]
  SPI_sendData(cfaf, y&0x00FF); //Start LSB = YS[ 7:0]
  SPI_sendData(cfaf, 0x01);     //End MSB   = YE[15:8] 320-1
  SPI_sendData(cfaf, 0x3F);     //End LSB   = YE[ 7:0]
  
  SPI_sendCommand(cfaf, ST7789_2C_RAMWR); //write data
}
	
	
void Set_Cursor(cfaf_t* cfaf, uint8_t x, uint8_t y){
  cfaf->cursor_x = x;
	cfaf->cursor_y = y;	
}

void Fill_LCD(cfaf_t* cfaf, uint8_t R, uint8_t G, uint8_t B){
  register int i;
  Set_LCD_for_write_at_X_Y(cfaf, 0, 0);

  //Fill display with a given RGB value
  for (i = 0; i < (320UL * 240UL); i++)
    {
    SPI_sendData(cfaf, B); //Blue
    SPI_sendData(cfaf, G); //Green
    SPI_sendData(cfaf, R); //Red
    }
  }
	
	
void Put_Pixel(cfaf_t* cfaf, int x, int y, uint8_t R, uint8_t G, uint8_t B){
  Set_LCD_for_write_at_X_Y(cfaf, x, y);
  //Write the single pixel's worth of data
  SPI_sendData(cfaf, B); //Blue
  SPI_sendData(cfaf, G); //Green
  SPI_sendData(cfaf, R); //Red
}

void Draw_Line(cfaf_t* cfaf, int x0, int y0, int x1, int y1, color_t color){
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      Put_Pixel(cfaf, y0, x0, color.r, color.g, color.b);
    } else {
      Put_Pixel(cfaf, x0, y0, color.r, color.g, color.b);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void Draw_FastVLine(cfaf_t* cfaf, int16_t x, int16_t y, int16_t h, color_t color) {
  Draw_Line(cfaf, x, y, x, y+h-1, color);
}


void Fill_Rect(cfaf_t* cfaf, int16_t x, int16_t y, int16_t w, int16_t h, color_t color) {
  // Update in subclasses if desired!
  for (int16_t i=x; i<x+w; i++) {
    Draw_FastVLine(cfaf, i, y, h, color);
  }
}

	
void LCD_Circle(cfaf_t* cfaf, int x0, int y0, int radius, uint8_t R, uint8_t G, uint8_t B){
  uint8_t x = radius;
  uint8_t y = 0;
  int16_t radiusError = 1 - (int16_t) x;

  while (x >= y)
    {
    //11 O'Clock
    Put_Pixel(cfaf, x0 - y, y0 + x, R, G, B);
    //1 O'Clock
    Put_Pixel(cfaf, x0 + y, y0 + x, R, G, B);
    //10 O'Clock
    Put_Pixel(cfaf, x0 - x, y0 + y, R, G, B);
    //2 O'Clock
    Put_Pixel(cfaf, x0 + x, y0 + y, R, G, B);
    //8 O'Clock
    Put_Pixel(cfaf, x0 - x, y0 - y, R, G, B);
    //4 O'Clock
    Put_Pixel(cfaf, x0 + x, y0 - y, R, G, B);
    //7 O'Clock
    Put_Pixel(cfaf, x0 - y, y0 - x, R, G, B);
    //5 O'Clock
    Put_Pixel(cfaf, x0 + y, y0 - x, R, G, B);

    y++;
    if (radiusError < 0)
      radiusError += (int16_t)(2 * y + 1);
    else
      {
      x--;
      radiusError += 2 * (((int16_t) y - (int16_t) x) + 1);
      }
    }
}
	

void Draw_Char(cfaf_t* cfaf, int x, int y, unsigned char c, color_t col, color_t bgcol, uint8_t size){
	if( (x >= cfaf->width)		|| // Clip right
      (y >= cfaf->height)		|| // Clip bottom
      ((x + 6 * size - 1) < 0)	|| // Clip left
      ((y + 8 * size - 1) < 0)
		) return;    // Clip top
    

  for (int8_t i=0; i<6; i++ ) {
    uint8_t line;
    if (i == 5) 
      line = 0x0;
    else 
      line = pgm_read_byte(font+(c*5)+i);
    for (int8_t j = 0; j<8; j++) {
      if (line & 0x1) {
        if (size == 1) // default size
          Put_Pixel(cfaf, x+i, y+j, col.r, col.g, col.b);
        else {  // big size
          Fill_Rect(cfaf, x+(i*size), y+(j*size), size, size, col);
        } 
      } else if (equal_col(bgcol, col)) {
        if (size == 1) // default size
          Put_Pixel(cfaf, x+i, y+j, bgcol.r, bgcol.g, bgcol.b);
        else {  // big size
          Fill_Rect(cfaf, x+i*size, y+j*size, size, size, bgcol);
        } 	
      }
      line >>= 1;
    }
  }
}
	
		
void Char_Write(cfaf_t* cfaf, uint8_t c) {
  if (c == '\n') {
    cfaf->cursor_y += cfaf->text_size * 8;
    cfaf->cursor_x = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    Draw_Char(cfaf, cfaf->cursor_x, cfaf->cursor_y, c, cfaf->text_color, cfaf->text_bgcolor, cfaf->text_size);
    cfaf->cursor_x = ( cfaf->cursor_x + cfaf->text_size*6 ) % cfaf->width;
    if (cfaf->wrap && (cfaf->cursor_x > (cfaf->width - cfaf->text_size*6))) {
      cfaf->cursor_y += cfaf->text_size*8;
      cfaf->cursor_x = 0;
    }
  }
}

void Write_Text(cfaf_t* cfaf, const char* text, uint8_t len){
	uint8_t i=0;
	while(i < len){
		Char_Write(cfaf, text[i]);
		i++;
	}
}

