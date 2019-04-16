#ifdef RA8875_USE

#include "ra8875.h"
#include "nrf_log.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_systick.h"


#define SPI_MISO_PIN 	30
#define SPI_MOSI_PIN 	29
#define SPI_SCK_PIN 	28
#define SPI_SS_PIN 		31
#define RESET_PIN			27

static uint8_t   m_tx_buf[64];           /**< TX buffer. */
static uint8_t   m_rx_buf[64];    /**< RX buffer. */

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(1);  /**< SPI instance. */

void spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context){}

// Initialize display object;	
ra8875_t display = {0};



/**************************************************************************/
/*!
      Constructor for a new RA8875 instance

      @args CS[in]  Location of the SPI chip select pin
      @args RST[in] Location of the reset pin
*/
/**************************************************************************/
void 	init_display (ra8875_t* disp, uint8_t mosi, uint8_t miso, uint8_t sck, uint8_t cs, uint8_t rst){
  disp->_cs = cs;
	disp->_rst = rst;
	disp->_mosi = mosi;
	disp->_miso = miso;
	disp->_sck = sck;
}

/**************************************************************************/
/*!
      Initialises the LCD driver and any HW required by the display

      @args s[in] The display size, which can be either:
                  'RA8875_480x272' (4.3" displays) r
                  'RA8875_800x480' (5" and 7" displays)
*/
/**************************************************************************/
bool display_begin(ra8875_t* disp, enum RA8875sizes s){
  disp->_size = s;

  if (disp->_size == RA8875_480x272) {
    disp->_width = 480;
    disp->_height = 272;
  }
  else if (disp->_size == RA8875_800x480) {
    disp->_width = 800;
    disp->_height = 480;
  }
  else {
    return false;
  }
	
	ret_code_t err_code;
	
	// initialize rst pin
	nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	err_code = nrf_drv_gpiote_in_init(disp->_rst, &in_config, NULL);
	APP_ERROR_CHECK(err_code);
	
	// initialize cs pin
	nrf_drv_gpiote_in_config_t in_config_2 = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
	err_code = nrf_drv_gpiote_in_init(disp->_cs, &in_config_2, NULL);
	APP_ERROR_CHECK(err_code);
	
	nrf_drv_gpiote_out_set(disp->_cs);
	nrf_drv_gpiote_out_clear(disp->_rst);
	nrfx_systick_delay_ms(100);
	nrf_drv_gpiote_out_set(disp->_rst);
	nrfx_systick_delay_ms(100);

	nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.ss_pin   = disp->_cs;
	spi_config.miso_pin = disp->_miso;
	spi_config.mosi_pin = disp->_mosi;
	spi_config.sck_pin  = disp->_sck;
	
	err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
	NRF_LOG_INFO("SPI : 0x%04X", err_code);
	APP_ERROR_CHECK(err_code);
	NRF_LOG_INFO("SPI started.");
	
  uint8_t x = readReg(disp, 0);
	NRF_LOG_INFO("X: 0x%02X", x);
	if (x != 0x75) {
		NRF_LOG_INFO("Error initializing display.");
		return false;
	}
	NRF_LOG_INFO("Display initialization complete.");

  /*initialize(disp);
*/
  return true;
}

/************************* Initialization *********************************/

/**************************************************************************/
/*!
      Performs a SW-based reset of the RA8875
*/
/**************************************************************************/
void softReset(ra8875_t* disp) {
  writeCommand(disp, RA8875_PWRR);
  writeData(disp, RA8875_PWRR_SOFTRESET);
  writeData(disp, RA8875_PWRR_NORMAL);
  nrfx_systick_delay_ms(1);
}

/**************************************************************************/
/*!
      Initialise the PLL
*/
/**************************************************************************/
void PLLinit(ra8875_t* disp) {
  if (disp->_size == RA8875_480x272) {
    writeReg(disp, RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
    nrfx_systick_delay_ms(1);
    writeReg(disp, RA8875_PLLC2, RA8875_PLLC2_DIV4);
    nrfx_systick_delay_ms(1);
  }
  else /* (_size == RA8875_800x480) */ {
    writeReg(disp, RA8875_PLLC1, RA8875_PLLC1_PLLDIV1 + 10);
    nrfx_systick_delay_ms(1);
    writeReg(disp, RA8875_PLLC2, RA8875_PLLC2_DIV4);
    nrfx_systick_delay_ms(1);
  }
}

/**************************************************************************/
/*!
      Initialises the driver IC (clock setup, etc.)
*/
/**************************************************************************/
void initialize(ra8875_t* disp) {
  PLLinit(disp);
  writeReg(disp, RA8875_SYSR, RA8875_SYSR_16BPP | RA8875_SYSR_MCU8);

  /* Timing values */
  uint8_t pixclk;
  uint8_t hsync_start;
  uint8_t hsync_pw;
  uint8_t hsync_finetune;
  uint8_t hsync_nondisp;
  uint8_t vsync_pw;
  uint16_t vsync_nondisp;
  uint16_t vsync_start;

  /* Set the correct values for the display being used */
  if (disp->_size == RA8875_480x272)
  {
    pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_4CLK;
    hsync_nondisp   = 10;
    hsync_start     = 8;
    hsync_pw        = 48;
    hsync_finetune  = 0;
    vsync_nondisp   = 3;
    vsync_start     = 8;
    vsync_pw        = 10;
  }
  else // (_size == RA8875_800x480)
  {
    pixclk          = RA8875_PCSR_PDATL | RA8875_PCSR_2CLK;
    hsync_nondisp   = 26;
    hsync_start     = 32;
    hsync_pw        = 96;
    hsync_finetune  = 0;
    vsync_nondisp   = 32;
    vsync_start     = 23;
    vsync_pw        = 2;
  }

  writeReg(disp, RA8875_PCSR, pixclk);
  nrfx_systick_delay_ms(1);

  /* Horizontal settings registers */
  writeReg(disp, RA8875_HDWR, (disp->_width / 8) - 1);                          // H width: (HDWR + 1) * 8 = 480
  writeReg(disp, RA8875_HNDFTR, RA8875_HNDFTR_DE_HIGH + hsync_finetune);
  writeReg(disp, RA8875_HNDR, (hsync_nondisp - hsync_finetune - 2)/8);    // H non-display: HNDR * 8 + HNDFTR + 2 = 10
  writeReg(disp, RA8875_HSTR, hsync_start/8 - 1);                         // Hsync start: (HSTR + 1)*8
  writeReg(disp, RA8875_HPWR, RA8875_HPWR_LOW + (hsync_pw/8 - 1));        // HSync pulse width = (HPWR+1) * 8

  /* Vertical settings registers */
  writeReg(disp, RA8875_VDHR0, (uint16_t)(disp->_height - 1) & 0xFF);
  writeReg(disp, RA8875_VDHR1, (uint16_t)(disp->_height - 1) >> 8);
  writeReg(disp, RA8875_VNDR0, vsync_nondisp-1);                          // V non-display period = VNDR + 1
  writeReg(disp, RA8875_VNDR1, vsync_nondisp >> 8);
  writeReg(disp, RA8875_VSTR0, vsync_start-1);                            // Vsync start position = VSTR + 1
  writeReg(disp, RA8875_VSTR1, vsync_start >> 8);
  writeReg(disp, RA8875_VPWR, RA8875_VPWR_LOW + vsync_pw - 1);            // Vsync pulse width = VPWR + 1

  /* Set active window X */
  writeReg(disp, RA8875_HSAW0, 0);                                        // horizontal start point
  writeReg(disp, RA8875_HSAW1, 0);
  writeReg(disp, RA8875_HEAW0, (uint16_t)(disp->_width - 1) & 0xFF);            // horizontal end point
  writeReg(disp, RA8875_HEAW1, (uint16_t)(disp->_width - 1) >> 8);

  /* Set active window Y */
  writeReg(disp, RA8875_VSAW0, 0);                                        // vertical start point
  writeReg(disp, RA8875_VSAW1, 0);
  writeReg(disp, RA8875_VEAW0, (uint16_t)(disp->_height - 1) & 0xFF);           // horizontal end point
  writeReg(disp, RA8875_VEAW1, (uint16_t)(disp->_height - 1) >> 8);

  /* ToDo: Setup touch panel? */

  /* Clear the entire window */
  writeReg(disp, RA8875_MCLR, RA8875_MCLR_START | RA8875_MCLR_FULL);
  nrfx_systick_delay_ms(500);
}

/**************************************************************************/
/*!
      Returns the display width in pixels

      @returns  The 1-based display width in pixels
*/
/**************************************************************************/
uint16_t width(ra8875_t* disp) { return disp->_width; }

/**************************************************************************/
/*!
      Returns the display height in pixels

      @returns  The 1-based display height in pixels
*/
/**************************************************************************/
uint16_t height(ra8875_t* disp) { return disp->_height; }

/************************* Text Mode ***********************************/

/**************************************************************************/
/*!
      Sets the display in text mode (as opposed to graphics mode)
*/
/**************************************************************************/
void textMode(ra8875_t* disp)
{
  /* Set text mode */
  writeCommand(disp, RA8875_MWCR0);
  uint8_t temp = readData(disp);
  temp |= RA8875_MWCR0_TXTMODE; // Set bit 7
  writeData(disp, temp);

  /* Select the internal (ROM) font */
  writeCommand(disp, 0x21);
  temp = readData(disp);
  temp &= ~((1<<7) | (1<<5)); // Clear bits 7 and 5
  writeData(disp, temp);
}

/**************************************************************************/
/*!
      Sets the display in text mode (as opposed to graphics mode)

      @args x[in] The x position of the cursor (in pixels, 0..1023)
      @args y[in] The y position of the cursor (in pixels, 0..511)
*/
/**************************************************************************/
void textSetCursor(ra8875_t* disp, uint16_t x, uint16_t y)
{
  /* Set cursor location */
  writeCommand(disp, 0x2A);
  writeData(disp, x & 0xFF);
  writeCommand(disp, 0x2B);
  writeData(disp, x >> 8);
  writeCommand(disp, 0x2C);
  writeData(disp, y & 0xFF);
  writeCommand(disp, 0x2D);
  writeData(disp, y >> 8);
}

/**************************************************************************/
/*!
      Sets the fore and background color when rendering text

      @args foreColor[in] The RGB565 color to use when rendering the text
      @args bgColor[in]   The RGB565 colot to use for the background
*/
/**************************************************************************/
void textColor(ra8875_t* disp, uint16_t foreColor, uint16_t bgColor)
{
  /* Set Fore Color */
  writeCommand(disp, 0x63);
  writeData(disp, (foreColor & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (foreColor & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (foreColor & 0x001f));

  /* Set Background Color */
  writeCommand(disp, 0x60);
  writeData(disp, (bgColor & 0xf800) >> 11);
  writeCommand(disp, 0x61);
  writeData(disp, (bgColor & 0x07e0) >> 5);
  writeCommand(disp, 0x62);
  writeData(disp, (bgColor & 0x001f));

  /* Clear transparency flag */
  writeCommand(disp, 0x22);
  uint8_t temp = readData(disp);
  temp &= ~(1<<6); // Clear bit 6
  writeData(disp, temp);
}

/**************************************************************************/
/*!
      Sets the fore color when rendering text with a transparent bg

      @args foreColor[in] The RGB565 color to use when rendering the text
*/
/**************************************************************************/
void textTransparent(ra8875_t* disp, uint16_t foreColor)
{
  /* Set Fore Color */
  writeCommand(disp, 0x63);
  writeData(disp, (foreColor & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (foreColor & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (foreColor & 0x001f));

  /* Set transparency flag */
  writeCommand(disp, 0x22);
  uint8_t temp = readData(disp);
  temp |= (1<<6); // Set bit 6
  writeData(disp, temp);
}

/**************************************************************************/
/*!
      Sets the text enlarge settings, using one of the following values:

      0 = 1x zoom
      1 = 2x zoom
      2 = 3x zoom
      3 = 4x zoom

      @args scale[in]   The zoom factor (0..3 for 1-4x zoom)
*/
/**************************************************************************/
void textEnlarge(ra8875_t* disp, uint8_t scale)
{
  if (scale > 3) scale = 3;

  /* Set font size flags */
  writeCommand(disp, 0x22);
  uint8_t temp = readData(disp);
  temp &= ~(0xF); // Clears bits 0..3
  temp |= scale << 2;
  temp |= scale;
  writeData(disp, temp);

  disp->_textScale = scale;
}

/**************************************************************************/
/*!
      Renders some text on the screen when in text mode

      @args buffer[in]    The buffer containing the characters to render
      @args len[in]       The size of the buffer in bytes
*/
/**************************************************************************/
void textWrite(ra8875_t* disp, const char* buffer, uint16_t len)
{
  if (len == 0) len = strlen(buffer);
  writeCommand(disp, RA8875_MRWC);
  for (uint16_t i=0;i<len;i++)
  {
    writeData(disp, buffer[i]);
#if defined(__arm__)
    // This delay is needed with textEnlarge(1) because
    // Teensy 3.X is much faster than Arduino Uno
    if (disp->_textScale > 0) nrfx_systick_delay_ms(1);
#else
    // For others, delay starting with textEnlarge(2)
    if (_textScale > 1) delay(1);
#endif
  }
}

/************************* Graphics ***********************************/

/**************************************************************************/
/*!
      Sets the display in graphics mode (as opposed to text mode)
*/
/**************************************************************************/
void graphicsMode(ra8875_t* disp) {
  writeCommand(disp, RA8875_MWCR0);
  uint8_t temp = readData(disp);
  temp &= ~RA8875_MWCR0_TXTMODE; // bit #7
  writeData(disp, temp);
}

/**************************************************************************/
/*!
      Waits for screen to finish by polling the status!
*/
/**************************************************************************/
bool waitPoll(ra8875_t* disp, uint8_t regname, uint8_t waitflag) {
  /* Wait for the command to finish */
  while (1)
  {
    uint8_t temp = readReg(disp, regname);
    if (!(temp & waitflag))
      return true;
  }
  return false; // MEMEFIX: yeah i know, unreached! - add timeout?
}


/**************************************************************************/
/*!
      Sets the current X/Y position on the display before drawing

      @args x[in] The 0-based x location
      @args y[in] The 0-base y location
*/
/**************************************************************************/
void setXY(ra8875_t* disp, uint16_t x, uint16_t y) {
  writeReg(disp, RA8875_CURH0, x);
  writeReg(disp, RA8875_CURH1, x >> 8);
  writeReg(disp, RA8875_CURV0, y);
  writeReg(disp, RA8875_CURV1, y >> 8);
}


/**************************************************************************/
/*!

*/
/**************************************************************************/
void fillRect(ra8875_t* disp) {
  writeCommand(disp, RA8875_DCR);
  writeData(disp, RA8875_DCR_LINESQUTRI_STOP | RA8875_DCR_DRAWSQUARE);
  writeData(disp, RA8875_DCR_LINESQUTRI_START | RA8875_DCR_FILL | RA8875_DCR_DRAWSQUARE);
}

/**************************************************************************/
/*!
      Draws a single pixel at the specified location

      @args x[in]     The 0-based x location
      @args y[in]     The 0-base y location
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawPixel(ra8875_t* disp, int16_t x, int16_t y, uint16_t color)
{
  writeReg(disp, RA8875_CURH0, x);
  writeReg(disp, RA8875_CURH1, x >> 8);
  writeReg(disp, RA8875_CURV0, y);
  writeReg(disp, RA8875_CURV1, y >> 8);
  writeCommand(disp, RA8875_MRWC);
	
	uint8_t dat[3];
	dat[0] = RA8875_DATAWRITE;
	dat[1] = color >> 8;
	dat[2] = color;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, dat, 3, m_rx_buf, 3));
}

/**************************************************************************/
/*!
 Draws a series of pixels at the specified location without the overhead

 @args p[in]     An array of RGB565 color pixels
 @args num[in]   The number of the pixels to draw
 @args x[in]     The 0-based x location
 @args y[in]     The 0-base y location
 */
/**************************************************************************/
void drawPixels(ra8875_t* disp, uint16_t * p, uint32_t num, int16_t x, int16_t y)
{
    writeReg(disp, RA8875_CURH0, x);
    writeReg(disp, RA8875_CURH1, x >> 8);
    writeReg(disp, RA8875_CURV0, y);
    writeReg(disp, RA8875_CURV1, y >> 8);
    writeCommand(disp, RA8875_MRWC);
	
		uint16_t tlen = num*2+1;
	
		uint8_t dat[tlen];
	  dat[0] = RA8875_DATAWRITE;
		uint16_t ptr = 1;
	  while (num--) {
			dat[ptr++] = (*p >> 8);
			dat[ptr++] = (*p & 0xFF);
			p++;
    }
	 
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, dat, tlen, m_rx_buf, tlen));
}

/**************************************************************************/
/*!
      Draws a HW accelerated line on the display

      @args x0[in]    The 0-based starting x location
      @args y0[in]    The 0-base starting y location
      @args x1[in]    The 0-based ending x location
      @args y1[in]    The 0-base ending y location
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawLine(ra8875_t* disp, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  /* Set X */
  writeCommand(disp, 0x91);
  writeData(disp, x0);
  writeCommand(disp, 0x92);
  writeData(disp, x0 >> 8);

  /* Set Y */
  writeCommand(disp, 0x93);
  writeData(disp, y0);
  writeCommand(disp, 0x94);
  writeData(disp, y0 >> 8);

  /* Set X1 */
  writeCommand(disp, 0x95);
  writeData(disp, x1);
  writeCommand(disp, 0x96);
  writeData(disp, (x1) >> 8);

  /* Set Y1 */
  writeCommand(disp, 0x97);
  writeData(disp, y1);
  writeCommand(disp, 0x98);
  writeData(disp, (y1) >> 8);

  /* Set Color */
  writeCommand(disp, 0x63);
  writeData(disp, (color & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (color & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (color & 0x001f));

  /* Draw! */
  writeCommand(disp, RA8875_DCR);
  writeData(disp, 0x80);

  /* Wait for the command to finish */
  waitPoll(disp, RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void drawFastVLine(ra8875_t* disp, int16_t x, int16_t y, int16_t h, uint16_t color)
{
  drawLine(disp, x, y, x, y+h, color);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void drawFastHLine(ra8875_t* disp, int16_t x, int16_t y, int16_t w, uint16_t color)
{
  drawLine(disp, x, y, x+w, y, color);
}

/**************************************************************************/
/*!
      Draws a HW accelerated rectangle on the display

      @args x[in]     The 0-based x location of the top-right corner
      @args y[in]     The 0-based y location of the top-right corner
      @args w[in]     The rectangle width
      @args h[in]     The rectangle height
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawRect(ra8875_t* disp, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  rectHelper(disp, x, y, x+w, y+h, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled rectangle on the display

      @args x[in]     The 0-based x location of the top-right corner
      @args y[in]     The 0-based y location of the top-right corner
      @args w[in]     The rectangle width
      @args h[in]     The rectangle height
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillRect_2(ra8875_t* disp, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
  rectHelper(disp, x, y, x+w, y+h, color, true);
}

/**************************************************************************/
/*!
      Fills the screen with the spefied RGB565 color

      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillScreen(ra8875_t* disp, uint16_t color)
{
  rectHelper(disp, 0, 0, disp->_width-1, disp->_height-1, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated circle on the display

      @args x[in]     The 0-based x location of the center of the circle
      @args y[in]     The 0-based y location of the center of the circle
      @args w[in]     The circle's radius
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawCircle(ra8875_t* disp, int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  circleHelper(disp, x0, y0, r, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled circle on the display

      @args x[in]     The 0-based x location of the center of the circle
      @args y[in]     The 0-based y location of the center of the circle
      @args w[in]     The circle's radius
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillCircle(ra8875_t* disp, int16_t x0, int16_t y0, int16_t r, uint16_t color)
{
  circleHelper(disp, x0, y0, r, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated triangle on the display

      @args x0[in]    The 0-based x location of point 0 on the triangle
      @args y0[in]    The 0-based y location of point 0 on the triangle
      @args x1[in]    The 0-based x location of point 1 on the triangle
      @args y1[in]    The 0-based y location of point 1 on the triangle
      @args x2[in]    The 0-based x location of point 2 on the triangle
      @args y2[in]    The 0-based y location of point 2 on the triangle
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawTriangle(ra8875_t* disp, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  triangleHelper(disp, x0, y0, x1, y1, x2, y2, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled triangle on the display

      @args x0[in]    The 0-based x location of point 0 on the triangle
      @args y0[in]    The 0-based y location of point 0 on the triangle
      @args x1[in]    The 0-based x location of point 1 on the triangle
      @args y1[in]    The 0-based y location of point 1 on the triangle
      @args x2[in]    The 0-based x location of point 2 on the triangle
      @args y2[in]    The 0-based y location of point 2 on the triangle
      @args color[in] The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillTriangle(ra8875_t* disp, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
  triangleHelper(disp, x0, y0, x1, y1, x2, y2, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated ellipse on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawEllipse(ra8875_t* disp, int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
  ellipseHelper(disp, xCenter, yCenter, longAxis, shortAxis, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled ellipse on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillEllipse(ra8875_t* disp, int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color)
{
  ellipseHelper(disp, xCenter, yCenter, longAxis, shortAxis, color, true);
}

/**************************************************************************/
/*!
      Draws a HW accelerated curve on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args curvePart[in] The corner to draw, where in clock-wise motion:
                            0 = 180-270°
                            1 = 270-0°
                            2 = 0-90°
                            3 = 90-180°
      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void drawCurve(ra8875_t* disp, int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  curveHelper(disp, xCenter, yCenter, longAxis, shortAxis, curvePart, color, false);
}

/**************************************************************************/
/*!
      Draws a HW accelerated filled curve on the display

      @args xCenter[in]   The 0-based x location of the ellipse's center
      @args yCenter[in]   The 0-based y location of the ellipse's center
      @args longAxis[in]  The size in pixels of the ellipse's long axis
      @args shortAxis[in] The size in pixels of the ellipse's short axis
      @args curvePart[in] The corner to draw, where in clock-wise motion:
                            0 = 180-270°
                            1 = 270-0°
                            2 = 0-90°
                            3 = 90-180°
      @args color[in]     The RGB565 color to use when drawing the pixel
*/
/**************************************************************************/
void fillCurve(ra8875_t* disp, int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color)
{
  curveHelper(disp, xCenter, yCenter, longAxis, shortAxis, curvePart, color, true);
}

/**************************************************************************/
/*!
      Helper function for higher level circle drawing code
*/
/**************************************************************************/
void circleHelper(ra8875_t* disp, int16_t x0, int16_t y0, int16_t r, uint16_t color, bool filled)
{
  /* Set X */
  writeCommand(disp, 0x99);
  writeData(disp, x0);
  writeCommand(disp, 0x9a);
  writeData(disp, x0 >> 8);

  /* Set Y */
  writeCommand(disp, 0x9b);
  writeData(disp, y0);
  writeCommand(disp, 0x9c);
  writeData(disp, y0 >> 8);

  /* Set Radius */
  writeCommand(disp, 0x9d);
  writeData(disp, r);

  /* Set Color */
  writeCommand(disp, 0x63);
  writeData(disp, (color & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (color & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (color & 0x001f));

  /* Draw! */
  writeCommand(disp, RA8875_DCR);
  if (filled)
  {
    writeData(disp, RA8875_DCR_CIRCLE_START | RA8875_DCR_FILL);
  }
  else
  {
    writeData(disp, RA8875_DCR_CIRCLE_START | RA8875_DCR_NOFILL);
  }

  /* Wait for the command to finish */
  waitPoll(disp, RA8875_DCR, RA8875_DCR_CIRCLE_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level rectangle drawing code
*/
/**************************************************************************/
void rectHelper(ra8875_t* disp, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color, bool filled)
{
  /* Set X */
  writeCommand(disp, 0x91);
  writeData(disp, x);
  writeCommand(disp, 0x92);
  writeData(disp, x >> 8);

  /* Set Y */
  writeCommand(disp, 0x93);
  writeData(disp, y);
  writeCommand(disp, 0x94);
  writeData(disp, y >> 8);

  /* Set X1 */
  writeCommand(disp, 0x95);
  writeData(disp, w);
  writeCommand(disp, 0x96);
  writeData(disp, (w) >> 8);

  /* Set Y1 */
  writeCommand(disp, 0x97);
  writeData(disp, h);
  writeCommand(disp, 0x98);
  writeData(disp, (h) >> 8);

  /* Set Color */
  writeCommand(disp, 0x63);
  writeData(disp, (color & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (color & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (color & 0x001f));

  /* Draw! */
  writeCommand(disp, RA8875_DCR);
  if (filled)
  {
    writeData(disp, 0xB0);
  }
  else
  {
    writeData(disp, 0x90);
  }

  /* Wait for the command to finish */
  waitPoll(disp, RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level triangle drawing code
*/
/**************************************************************************/
void triangleHelper(ra8875_t* disp, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, bool filled)
{
  /* Set Point 0 */
  writeCommand(disp, 0x91);
  writeData(disp, x0);
  writeCommand(disp, 0x92);
  writeData(disp, x0 >> 8);
  writeCommand(disp, 0x93);
  writeData(disp, y0);
  writeCommand(disp, 0x94);
  writeData(disp, y0 >> 8);

  /* Set Point 1 */
  writeCommand(disp, 0x95);
  writeData(disp, x1);
  writeCommand(disp, 0x96);
  writeData(disp, x1 >> 8);
  writeCommand(disp, 0x97);
  writeData(disp, y1);
  writeCommand(disp, 0x98);
  writeData(disp, y1 >> 8);

  /* Set Point 2 */
  writeCommand(disp, 0xA9);
  writeData(disp, x2);
  writeCommand(disp, 0xAA);
  writeData(disp, x2 >> 8);
  writeCommand(disp, 0xAB);
  writeData(disp, y2);
  writeCommand(disp, 0xAC);
  writeData(disp, y2 >> 8);

  /* Set Color */
  writeCommand(disp, 0x63);
  writeData(disp, (color & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (color & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (color & 0x001f));

  /* Draw! */
  writeCommand(disp, RA8875_DCR);
  if (filled)
  {
    writeData(disp, 0xA1);
  }
  else
  {
    writeData(disp, 0x81);
  }

  /* Wait for the command to finish */
  waitPoll(disp, RA8875_DCR, RA8875_DCR_LINESQUTRI_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level ellipse drawing code
*/
/**************************************************************************/
void ellipseHelper(ra8875_t* disp, int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint16_t color, bool filled)
{
  /* Set Center Point */
  writeCommand(disp, 0xA5);
  writeData(disp, xCenter);
  writeCommand(disp, 0xA6);
  writeData(disp, xCenter >> 8);
  writeCommand(disp, 0xA7);
  writeData(disp, yCenter);
  writeCommand(disp, 0xA8);
  writeData(disp, yCenter >> 8);

  /* Set Long and Short Axis */
  writeCommand(disp, 0xA1);
  writeData(disp, longAxis);
  writeCommand(disp, 0xA2);
  writeData(disp, longAxis >> 8);
  writeCommand(disp, 0xA3);
  writeData(disp, shortAxis);
  writeCommand(disp, 0xA4);
  writeData(disp, shortAxis >> 8);

  /* Set Color */
  writeCommand(disp, 0x63);
  writeData(disp, (color & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (color & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (color & 0x001f));

  /* Draw! */
  writeCommand(disp, 0xA0);
  if (filled)
  {
    writeData(disp, 0xC0);
  }
  else
  {
    writeData(disp, 0x80);
  }

  /* Wait for the command to finish */
  waitPoll(disp, RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/**************************************************************************/
/*!
      Helper function for higher level curve drawing code
*/
/**************************************************************************/
void curveHelper(ra8875_t* disp, int16_t xCenter, int16_t yCenter, int16_t longAxis, int16_t shortAxis, uint8_t curvePart, uint16_t color, bool filled)
{
  /* Set Center Point */
  writeCommand(disp, 0xA5);
  writeData(disp, xCenter);
  writeCommand(disp, 0xA6);
  writeData(disp, xCenter >> 8);
  writeCommand(disp, 0xA7);
  writeData(disp, yCenter);
  writeCommand(disp, 0xA8);
  writeData(disp, yCenter >> 8);

  /* Set Long and Short Axis */
  writeCommand(disp, 0xA1);
  writeData(disp, longAxis);
  writeCommand(disp, 0xA2);
  writeData(disp, longAxis >> 8);
  writeCommand(disp, 0xA3);
  writeData(disp, shortAxis);
  writeCommand(disp, 0xA4);
  writeData(disp, shortAxis >> 8);

  /* Set Color */
  writeCommand(disp, 0x63);
  writeData(disp, (color & 0xf800) >> 11);
  writeCommand(disp, 0x64);
  writeData(disp, (color & 0x07e0) >> 5);
  writeCommand(disp, 0x65);
  writeData(disp, (color & 0x001f));

  /* Draw! */
  writeCommand(disp, 0xA0);
  if (filled)
  {
    writeData(disp, 0xD0 | (curvePart & 0x03));
  }
  else
  {
    writeData(disp, 0x90 | (curvePart & 0x03));
  }

  /* Wait for the command to finish */
  waitPoll(disp, RA8875_ELLIPSE, RA8875_ELLIPSE_STATUS);
}

/************************* Mid Level ***********************************/

/**************************************************************************/
/*!

*/
/**************************************************************************/
void GPIOX(ra8875_t* disp, bool on) {
  if (on)
    writeReg(disp, RA8875_GPIOX, 1);
  else
    writeReg(disp, RA8875_GPIOX, 0);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void PWM1out(ra8875_t* disp, uint8_t p) {
  writeReg(disp, RA8875_P1DCR, p);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void PWM2out(ra8875_t* disp, uint8_t p) {
  writeReg(disp, RA8875_P2DCR, p);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void PWM1config(ra8875_t* disp, bool on, uint8_t clock) {
  if (on) {
    writeReg(disp, RA8875_P1CR, RA8875_P1CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(disp, RA8875_P1CR, RA8875_P1CR_DISABLE | (clock & 0xF));
  }
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void PWM2config(ra8875_t* disp, bool on, uint8_t clock) {
  if (on) {
    writeReg(disp, RA8875_P2CR, RA8875_P2CR_ENABLE | (clock & 0xF));
  } else {
    writeReg(disp, RA8875_P2CR, RA8875_P2CR_DISABLE | (clock & 0xF));
  }
}

/**************************************************************************/
/*!
      Enables or disables the on-chip touch screen controller
*/
/**************************************************************************/
void touchEnable(ra8875_t* disp, bool on)
{
  uint8_t   adcClk = (uint8_t) RA8875_TPCR0_ADCCLK_DIV4;

  if ( disp->_size == RA8875_800x480 ) //match up touch size with LCD size
    adcClk = (uint8_t) RA8875_TPCR0_ADCCLK_DIV16;

  if (on)
  {
    /* Enable Touch Panel (Reg 0x70) */
    writeReg(disp, RA8875_TPCR0, RA8875_TPCR0_ENABLE        |
                           RA8875_TPCR0_WAIT_4096CLK  |
                           RA8875_TPCR0_WAKEENABLE   |
                           adcClk); // 10mhz max!
    /* Set Auto Mode      (Reg 0x71) */
    writeReg(disp, RA8875_TPCR1, RA8875_TPCR1_AUTO    |
                           // RA8875_TPCR1_VREFEXT |
                           RA8875_TPCR1_DEBOUNCE);
    /* Enable TP INT */
    writeReg(disp, RA8875_INTC1, readReg(disp, RA8875_INTC1) | RA8875_INTC1_TP);
  }
  else
  {
    /* Disable TP INT */
    writeReg(disp, RA8875_INTC1, readReg(disp, RA8875_INTC1) & ~RA8875_INTC1_TP);
    /* Disable Touch Panel (Reg 0x70) */
    writeReg(disp, RA8875_TPCR0, RA8875_TPCR0_DISABLE);
  }
}

/**************************************************************************/
/*!
      Checks if a touch event has occured

      @returns  True is a touch event has occured (reading it via
                touchRead() will clear the interrupt in memory)
*/
/**************************************************************************/
bool touched(ra8875_t* disp)
{
  if (readReg(disp, RA8875_INTC2) & RA8875_INTC2_TP) return true;
  return false;
}

/**************************************************************************/
/*!
      Reads the last touch event

      @args x[out]  Pointer to the uint16_t field to assign the raw X value
      @args y[out]  Pointer to the uint16_t field to assign the raw Y value

      @note Calling this function will clear the touch panel interrupt on
            the RA8875, resetting the flag used by the 'touched' function
*/
/**************************************************************************/
bool touchRead(ra8875_t* disp, uint16_t *x, uint16_t *y)
{
  uint16_t tx, ty;
  uint8_t temp;

  tx = readReg(disp, RA8875_TPXH);
  ty = readReg(disp, RA8875_TPYH);
  temp = readReg(disp, RA8875_TPXYL);
  tx <<= 2;
  ty <<= 2;
  tx |= temp & 0x03;        // get the bottom x bits
  ty |= (temp >> 2) & 0x03; // get the bottom y bits

  *x = tx;
  *y = ty;

  /* Clear TP INT Status */
  writeReg(disp, RA8875_INTC2, RA8875_INTC2_TP);

  return true;
}

/**************************************************************************/
/*!
      Turns the display on or off
*/
/**************************************************************************/
void displayOn(ra8875_t* disp, bool on)
{
 if (on)
   writeReg(disp, RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPON);
 else
   writeReg(disp, RA8875_PWRR, RA8875_PWRR_NORMAL | RA8875_PWRR_DISPOFF);
}

/**************************************************************************/
/*!
    Puts the display in sleep mode, or disables sleep mode if enabled
*/
/**************************************************************************/
void sleep(ra8875_t* disp, bool sleep)
{
 if (sleep)
   writeReg(disp, RA8875_PWRR, RA8875_PWRR_DISPOFF | RA8875_PWRR_SLEEP);
 else
   writeReg(disp, RA8875_PWRR, RA8875_PWRR_DISPOFF);
}

/************************* Low Level ***********************************/

/**************************************************************************/
/*!

*/
/**************************************************************************/
void  writeReg(ra8875_t* disp, uint8_t reg, uint8_t val)
{
  writeCommand(disp, reg);
  writeData(disp, val);
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
uint8_t  readReg(ra8875_t* disp, uint8_t reg)
{
  writeCommand(disp, reg);
  return readData(disp);
	
	/*uint8_t dat[4];
	dat[0] = RA8875_CMDWRITE;
	dat[1] = reg;
	dat[2] = RA8875_DATAREAD;
	dat[3] = 0x0;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, dat, 4, m_rx_buf, 4));
	NRF_LOG_INFO("data: 0x%02X 0x%02X 0x%02X  0x%02X", m_rx_buf[0], m_rx_buf[1], m_rx_buf[2], m_rx_buf[3]);
	return m_rx_buf[0];*/
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void  writeData(ra8875_t* disp, uint8_t d)
{
	uint8_t dat[2];
	dat[0] = RA8875_DATAWRITE;
	dat[1] = d;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, dat, 2, m_rx_buf, 2));
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
uint8_t  readData(ra8875_t* disp)
{
	memset(m_rx_buf, 0, 2);
	
	uint8_t dat[2];
	dat[0] = RA8875_DATAREAD;
	dat[1] = 0x0;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, dat, 2, m_rx_buf, 2));
	
  return m_rx_buf[1];
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
void  writeCommand(ra8875_t* disp, uint8_t d)
{	
	uint8_t dat[2];
	dat[0] = RA8875_CMDWRITE;
	dat[1] = d;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, dat, 2, m_rx_buf, 2));
}

/**************************************************************************/
/*!

*/
/**************************************************************************/
uint8_t  readStatus(ra8875_t* disp)
{
	memset(m_rx_buf, 0, 2);
	
	uint8_t dat[2];
	dat[0] = RA8875_CMDREAD;
	dat[1] = 0x0;
	
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, dat, 2, m_rx_buf, 2));
	
  return m_rx_buf[1];
}


#endif



