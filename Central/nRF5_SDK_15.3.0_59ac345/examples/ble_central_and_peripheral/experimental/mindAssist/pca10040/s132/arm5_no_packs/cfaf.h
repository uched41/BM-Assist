#ifndef	CFAF128128_H
#define CFAF128128_H

#include "nrf.h"
#include <stdbool.h>
#include "nrf_drv_spi.h"
#include "nrf_drv_systick.h"

#define delay(x) nrfx_systick_delay_ms(x)

#define SYNC_NONE   (0)
#define SYNC_MASTER (1)
#define SYNC_SLAVE  (2)

#define SYNC SYNC_NONE

// ref: https://www.crystalfontz.com/products/document/3277/ST7735_V2.1_20100505.pdf

// Color struct
typedef struct{
	uint8_t r, g, b;
}color_t;

// Display struct
typedef struct{
	uint8_t _cs, _reset, _mosi, _miso, _sck, _rs;
	color_t text_color;
	uint8_t text_size;
	uint8_t cursor_x, cursor_y;
	uint8_t width, height;
	color_t text_bgcolor;
	bool wrap;
}cfaf_t;

void SPI_sendCommand(cfaf_t* cfaf, uint8_t command);

void SPI_sendData(cfaf_t* cfaf, uint8_t data);

bool Initialize_Cfaf(cfaf_t* cfaf);

void Setup_Lcd(cfaf_t* cfaf);

void Set_LCD_for_write_at_X_Y(cfaf_t* cfaf, uint8_t x, uint8_t y);

void Fill_LCD(cfaf_t* cfaf, uint8_t R, uint8_t G, uint8_t B);

void Put_Pixel(cfaf_t* cfaf, uint8_t x, uint8_t y, uint8_t R, uint8_t G, uint8_t B);

void LCD_Circle(cfaf_t* cfaf, uint8_t x0, uint8_t y0, uint8_t radius, uint8_t R, uint8_t G, uint8_t B);

void SPI_send_pixels(cfaf_t* cfaf, uint8_t byte_count, uint8_t *data_ptr);
 
void Draw_Line(cfaf_t* cfaf, int16_t x0, int16_t y0, int16_t x1, int16_t y1, color_t color);

void Draw_FastVLine(cfaf_t* cfaf, int16_t x, int16_t y, int16_t h, color_t color);

void Draw_Char(cfaf_t* cfaf, uint8_t x, uint8_t y, unsigned char c, color_t col, color_t bgcol, uint8_t size);

void Write_Text(cfaf_t* cfaf, const char* text, uint8_t len);

void Char_Write(cfaf_t* cfaf, uint8_t c);

void Set_Cursor(cfaf_t* cfaf, uint8_t x, uint8_t y);

void On_Display(cfaf_t* cfaf);

void Off_Display(cfaf_t* cfaf);

void CLR_RS    (cfaf_t* cfaf);
void SET_RS    (cfaf_t* cfaf);
void CLR_RESET (cfaf_t* cfaf);
void SET_RESET (cfaf_t* cfaf);
void CLR_CS    (cfaf_t* cfaf);
void SET_CS    (cfaf_t* cfaf);
void CLR_MOSI  (cfaf_t* cfaf);
void SET_MOSI  (cfaf_t* cfaf);
void CLR_SCK   (cfaf_t* cfaf);
void SET_SCK   (cfaf_t* cfaf);


#define ST7735_SLPIN    (0x10)
#define ST7735_SLPOUT   (0x11)
#define ST7735_DISPON   (0x29)
#define ST7735_DISPOFF  (0x28)
#define ST7735_CASET    (0x2A)
#define ST7735_RASET    (0x2B)
#define ST7735_RAMWR    (0x2C)
#define ST7735_RAMRD    (0x2E)
#define ST7735_MADCTL   (0x36)
#define ST7735_COLMOD   (0x3A)
#define ST7735_FRMCTR1  (0xB1)
#define ST7735_FRMCTR2  (0xB2)
#define ST7735_FRMCTR3  (0xB3)
#define ST7735_INVCTR   (0xB4)
#define ST7735_PWCTR1   (0xC0)
#define ST7735_PWCTR2   (0xC1)
#define ST7735_PWCTR3   (0xC2)
#define ST7735_PWCTR4   (0xC3)
#define ST7735_PWCTR5   (0xC4)
#define ST7735_VMCTR1   (0xC5)
#define ST7735_GAMCTRP1 (0xE0)
#define ST7735_GAMCTRN1 (0xE1)


 
		
#endif