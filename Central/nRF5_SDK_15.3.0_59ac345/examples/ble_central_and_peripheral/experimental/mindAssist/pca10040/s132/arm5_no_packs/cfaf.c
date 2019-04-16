#include "cfaf.h"
#include "nrf_log.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "font.c"
#include <stdlib.h>

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define swap(a, b) { int16_t t = a; a = b; b = t; }
#define equal_col(a, c) (a.r == c.r && c.g == a.g && a.b == c.b)


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
	
	//Drive the ports to a reasonable starting state.
  CLR_RESET(cfaf);
  CLR_RS(cfaf);
  SET_CS(cfaf);
																
	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = cfaf->_cs;
	spi_config.mosi_pin = cfaf->_mosi;
	spi_config.sck_pin  = cfaf->_sck;
	
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

  SPI_sendCommand(cfaf, ST7735_SLPOUT);
  delay(120);

  SPI_sendCommand(cfaf, ST7735_FRMCTR1);//In normal mode(Full colors)
  SPI_sendData(cfaf, 0x02);//RTNB: set 1-line period
  SPI_sendData(cfaf, 0x35);//FPB:  front porch
  SPI_sendData(cfaf, 0x36);//BPB:  back porch

  SPI_sendCommand(cfaf, ST7735_FRMCTR2);//In Idle mode (8-colors)
  SPI_sendData(cfaf, 0x02);//RTNB: set 1-line period
  SPI_sendData(cfaf, 0x35);//FPB:  front porch
  SPI_sendData(cfaf, 0x36);//BPB:  back porch

  SPI_sendCommand(cfaf, ST7735_FRMCTR3);//In partial mode + Full colors
  SPI_sendData(cfaf, 0x02);//RTNC: set 1-line period
  SPI_sendData(cfaf, 0x35);//FPC:  front porch
  SPI_sendData(cfaf, 0x36);//BPC:  back porch
  SPI_sendData(cfaf, 0x02);//RTND: set 1-line period
  SPI_sendData(cfaf, 0x35);//FPD:  front porch
  SPI_sendData(cfaf, 0x36);//BPD:  back porch

  //INVCTR (B4h): Display Inversion Control
  SPI_sendCommand(cfaf, ST7735_INVCTR);
  SPI_sendData(cfaf, 0x07);
  
  //PWCTR1 (C0h): Power Control 1
  SPI_sendCommand(cfaf, ST7735_PWCTR1);
  SPI_sendData(cfaf, 0x02);
  SPI_sendData(cfaf, 0x02);
  SPI_sendCommand(cfaf, ST7735_PWCTR2);
  SPI_sendData(cfaf, 0xC5);
  SPI_sendCommand(cfaf, ST7735_PWCTR3);
  SPI_sendData(cfaf, 0x0D);
  SPI_sendData(cfaf, 0x00);
  SPI_sendCommand(cfaf, ST7735_PWCTR4);
  SPI_sendData(cfaf, 0x8D);
  SPI_sendData(cfaf, 0x1A);
  SPI_sendCommand(cfaf, ST7735_PWCTR5);
  SPI_sendData(cfaf, 0x8D);
  SPI_sendData(cfaf, 0xEE);
  SPI_sendCommand(cfaf, ST7735_VMCTR1);
  SPI_sendData(cfaf, 0x51);
  SPI_sendData(cfaf, 0x4D);
  SPI_sendCommand(cfaf, ST7735_GAMCTRP1);
  SPI_sendData(cfaf, 0x0a);
  SPI_sendData(cfaf, 0x1c);
  SPI_sendData(cfaf, 0x0c);
  SPI_sendData(cfaf, 0x14);
  SPI_sendData(cfaf, 0x33);
  SPI_sendData(cfaf, 0x2b);
  SPI_sendData(cfaf, 0x24);
  SPI_sendData(cfaf, 0x28);
  SPI_sendData(cfaf, 0x27);
  SPI_sendData(cfaf, 0x25);
  SPI_sendData(cfaf, 0x2C);
  SPI_sendData(cfaf, 0x39);
  SPI_sendData(cfaf, 0x00);
  SPI_sendData(cfaf, 0x05);
  SPI_sendData(cfaf, 0x03);
  SPI_sendData(cfaf, 0x0d);

  SPI_sendCommand(cfaf, ST7735_GAMCTRN1);
  SPI_sendData(cfaf, 0x0a);
  SPI_sendData(cfaf, 0x1c);
  SPI_sendData(cfaf, 0x0c);
  SPI_sendData(cfaf, 0x14);
  SPI_sendData(cfaf, 0x33);
  SPI_sendData(cfaf, 0x2b);
  SPI_sendData(cfaf, 0x24);
  SPI_sendData(cfaf, 0x28);
  SPI_sendData(cfaf, 0x27);
  SPI_sendData(cfaf, 0x25);
  SPI_sendData(cfaf, 0x2D);
  SPI_sendData(cfaf, 0x3a);
  SPI_sendData(cfaf, 0x00);
  SPI_sendData(cfaf, 0x05);
  SPI_sendData(cfaf, 0x03);
  SPI_sendData(cfaf, 0x0d);

  SPI_sendCommand(cfaf, ST7735_COLMOD);
  SPI_sendData(cfaf, 0x06);
  SPI_sendCommand(cfaf, ST7735_DISPON);//Display On
  delay(1);
	
  //MADCTL (36h): Memory Data Access Control
  SPI_sendCommand(cfaf, ST7735_MADCTL);
  SPI_sendData(cfaf, 0xc0);
	delay(10);
	Fill_LCD(cfaf, 0, 0, 0);
	delay(100);
	
	Off_Display(cfaf);
}

void Off_Display(cfaf_t* cfaf){
	SPI_sendCommand(cfaf, ST7735_DISPOFF);//Display Off
}

void On_Display(cfaf_t* cfaf){
	SPI_sendCommand(cfaf, ST7735_DISPON);//Display On
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

void Set_LCD_for_write_at_X_Y(cfaf_t* cfaf, uint8_t x, uint8_t y){	
  SPI_sendCommand(cfaf, ST7735_CASET); 
  SPI_sendData(cfaf, 0x00);     //Start MSB = XS[15:8]
  SPI_sendData(cfaf, 0x02 + x); //Start LSB = XS[ 7:0]
  SPI_sendData(cfaf, 0x00);     //End MSB   = XE[15:8]
  SPI_sendData(cfaf, 0x81);     //End LSB   = XE[ 7:0]
  
  SPI_sendCommand(cfaf, ST7735_RASET); //Row address set
  SPI_sendData(cfaf, 0x00);     //Start MSB = YS[15:8]
  SPI_sendData(cfaf, 0x01 + y); //Start LSB = YS[ 7:0]
  SPI_sendData(cfaf, 0x00);     //End MSB   = YE[15:8]
  SPI_sendData(cfaf, 0x80);     //End LSB   = YE[ 7:0]
  SPI_sendCommand(cfaf, ST7735_RAMWR); //write data
}
	
	
void Set_Cursor(cfaf_t* cfaf, uint8_t x, uint8_t y){
  cfaf->cursor_x = x;
	cfaf->cursor_y = y;	
}

void Fill_LCD(cfaf_t* cfaf, uint8_t R, uint8_t G, uint8_t B){
  register int i;
  Set_LCD_for_write_at_X_Y(cfaf, 0, 0);

  //Fill display with a given RGB value
  for (i = 0; i < (128 * 128); i++)
    {
    SPI_sendData(cfaf, B); //Blue
    SPI_sendData(cfaf, G); //Green
    SPI_sendData(cfaf, R); //Red
    }
  }
	
	
void Put_Pixel(cfaf_t* cfaf, uint8_t x, uint8_t y, uint8_t R, uint8_t G, uint8_t B){
  Set_LCD_for_write_at_X_Y(cfaf, x, y);
  //Write the single pixel's worth of data
  SPI_sendData(cfaf, B); //Blue
  SPI_sendData(cfaf, G); //Green
  SPI_sendData(cfaf, R); //Red
}

void Draw_Line(cfaf_t* cfaf, int16_t x0, int16_t y0, int16_t x1, int16_t y1, color_t color){
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

	
void LCD_Circle(cfaf_t* cfaf, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t R, uint8_t G, uint8_t B){
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
	

void Draw_Char(cfaf_t* cfaf, uint8_t x, uint8_t y, unsigned char c, color_t col, color_t bgcol, uint8_t size){
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

