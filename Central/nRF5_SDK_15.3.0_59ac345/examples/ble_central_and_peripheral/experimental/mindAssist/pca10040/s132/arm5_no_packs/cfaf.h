#ifndef	CFAF240320_H
#define CFAF240320_H

#include "nrf.h"
#include <stdbool.h>
#include "nrf_drv_spi.h"
#include "nrf_drv_systick.h"

#define delay(x) nrfx_systick_delay_ms(x)

#define CFAF240320V020T (0)
// IPS TFT Wide, all viewing angle -- more expensive
#define CFAF240320W020T (1)
// O-film TFT Wide angle, no inversion direction, lower cost than IPS
#define CFAF240320X020T (2)

#define SYNC_NONE   (0)
#define SYNC_MASTER (1)
#define SYNC_SLAVE  (2)

#define SYNC SYNC_NONE

#define SPI4Wire

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
	int cursor_x, cursor_y;
	int width, height;
	color_t text_bgcolor;
	bool wrap;
}cfaf_t;

void SPI_sendCommand(cfaf_t* cfaf, uint8_t command);

void SPI_sendData(cfaf_t* cfaf, uint8_t data);

bool Initialize_Cfaf(cfaf_t* cfaf);

void Setup_Lcd(cfaf_t* cfaf);

void Set_LCD_for_write_at_X_Y(cfaf_t* cfaf, int x, int y);

void Fill_LCD(cfaf_t* cfaf, uint8_t R, uint8_t G, uint8_t B);

void Put_Pixel(cfaf_t* cfaf, int x, int y, uint8_t R, uint8_t G, uint8_t B);

void LCD_Circle(cfaf_t* cfaf, int x0, int y0, int radius, uint8_t R, uint8_t G, uint8_t B);

void SPI_send_pixels(cfaf_t* cfaf, uint8_t byte_count, uint8_t *data_ptr);
 
void Draw_Line(cfaf_t* cfaf, int x0, int y0, int x1, int y1, color_t color);

void Draw_FastVLine(cfaf_t* cfaf, int16_t x, int16_t y, int16_t h, color_t color);

void Draw_Char(cfaf_t* cfaf, int x, int y, unsigned char c, color_t col, color_t bgcol, uint8_t size);

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


#define ST7789_00_NOP       (0x00)
#define ST7789_01_SWRESET   (0x01)
#define ST7789_04_RDDID     (0x04)
#define ST7789_09_RDDST     (0x09)
#define ST7789_0A_RDDPM     (0x0A)
#define ST7789_0B_RDDMADCTL (0x0B)
#define ST7789_0C_RDDCOLMOD (0x0C)
#define ST7789_0D_RDDIM     (0x0D)
#define ST7789_0E_RDDSM     (0x0E)
#define ST7789_0F_RDDSDR    (0x0F)
#define ST7789_10_SLPIN     (0x10)
#define ST7789_11_SLPOUT    (0x11)
#define ST7789_12_PTLON     (0x12)
#define ST7789_13_NORON     (0x13)
#define ST7789_20_INVOFF    (0x20)
#define ST7789_21_INVON     (0x21)
#define ST7789_26_GAMSET    (0x26)
#define ST7789_28_DISPOFF   (0x28)
#define ST7789_29_DISPON    (0x29)
#define ST7789_2A_CASET     (0x2A)
#define ST7789_2B_RASET     (0x2B)
#define ST7789_2C_RAMWR     (0x2C)
#define ST7789_2E_RAMRD     (0x2E)
#define ST7789_30_PTLAR     (0x30)
#define ST7789_33_VSCRDEF   (0x33)
#define ST7789_34_TEOFF     (0x34)
#define ST7789_35_TEON      (0x35)
#define ST7789_36_MADCTL    (0x36)
#define ST7789_37_VSCRSADD  (0x37)
#define ST7789_38_IDMOFF    (0x38)
#define ST7789_39_IDMON     (0x39)
#define ST7789_3A_COLMOD    (0x3A)
#define ST7789_3C_RAMWRC    (0x3C)
#define ST7789_3E_RAMRDC    (0x3E)
#define ST7789_44_TESCAN    (0x44)
#define ST7789_45_RDTESCAN  (0x45)
#define ST7789_51_WRDISBV   (0x51)
#define ST7789_52_RDDISBV   (0x52)
#define ST7789_53_WRCTRLD   (0x53)
#define ST7789_54_RDCTRLD   (0x54)
#define ST7789_55_WRCACE    (0x55)
#define ST7789_56_RDCABC    (0x56)
#define ST7789_5E_WRCABCMB  (0x5E)
#define ST7789_5F_RDCABCMB  (0x5F)
#define ST7789_68_RDABCSDR  (0x68)
#define ST7789_B0_RAMCTRL   (0xB0)
#define ST7789_B1_RGBCTRL   (0xB1)
#define ST7789_B2_PORCTRL   (0xB2)
#define ST7789_B3_FRCTRL1   (0xB3)
#define ST7789_B7_GCTRL     (0xB7)
#define ST7789_BA_DGMEN     (0xBA)
#define ST7789_BB_VCOMS     (0xBB)
#define ST7789_C0_LCMCTRL   (0xC0)
#define ST7789_C1_IDSET     (0xC1)
#define ST7789_C2_VDVVRHEN  (0xC2)
#define ST7789_C3_VRHS      (0xC3)
#define ST7789_C4_VDVSET    (0xC4)
#define ST7789_C5_VCMOFSET  (0xC5)
#define ST7789_C6_FRCTR2    (0xC6)
#define ST7789_C7_CABCCTRL  (0xC7)
#define ST7789_C8_REGSEL1   (0xC8)
#define ST7789_CA_REGSEL2   (0xCA)
#define ST7789_CC_PWMFRSEL  (0xCC)
#define ST7789_D0_PWCTRL1   (0xD0)
#define ST7789_D2_VAPVANEN  (0xD2)
#define ST7789_DA_RDID1     (0xDA)
#define ST7789_DB_RDID2     (0xDB)
#define ST7789_DC_RDID3     (0xDC)
#define ST7789_DF_CMD2EN    (0xDF)
#define ST7789_E0_PVGAMCTRL (0xE0)
#define ST7789_E1_NVGAMCTRL (0xE1)
#define ST7789_E2_DGMLUTR   (0xE2)
#define ST7789_E3_DGMLUTB   (0xE3)
#define ST7789_E4_GATECTRL  (0xE4)
#define ST7789_E8_PWCTRL2   (0xE8)
#define ST7789_E9_EQCTRL    (0xE9)
#define ST7789_EC_PROMCTRL  (0xEC)
#define ST7789_FA_PROMEN    (0xFA)
#define ST7789_FC_NVMSET    (0xFC)
#define ST7789_FE_PROMACT   (0xFE)
		
#endif