#include <SPI.h>
// C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SPI\src\SPI.cpp
// C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SPI\src\SPI.h

#include <SD.h>
// C:\Program Files (x86)\Arduino\libraries\SD\src\SD.cpp
// C:\Program Files (x86)\Arduino\libraries\SD\src\SD.h

//#include <util/delay.h>
//#include <avr/pgmspace.h>
//============================================================================
// DISPLAYS SUPPORTERD BY THIS CODE
//----------------------------------------------------------------------------
// Normal TN TFT Good viewing angle but has a definite inversion direction.
#define CFAF240320V020T (0)
// IPS TFT Wide, all viewing angle -- more expensive
#define CFAF240320W020T (1)
// O-film TFT Wide angle, no inversion direction, lower cost than IPS
#define CFAF240320X020T (2)
//============================================================================
// CHOOSE YOUR DISPLAY HERE
#define display CFAF240320X020T
//============================================================================
// This code supports a rudimentary touch screen demonstration. There are two
// portions, the first is a very simple calibration.
//
// To calibrate set TOUCH_ENABLED and FIND_MIN_MAX, upload and enable the
// serial monitor. Then use the stylus to draw entirely across the X and Y
// axes. Then pick the values out of the seial monitor and use them to set
// Xmin, Xmax, Ymin and Ymax.
//
// Once those are determined, then clear FIND_MIN_MAX, upload again and
// you will be able to draw on the screen. 
//
#define TOUCH_ENABLED 0
#define FIND_MIN_MAX  0
//
#if(FIND_MIN_MAX)
  uint16_t Xmin=1023;
  uint16_t Xmax=0;
  uint16_t Ymin=1023;
  uint16_t Ymax=0;
#else
  //Copied from the serial console window
  uint16_t Xmin=122;
  uint16_t Xmax=892;
  uint16_t Ymin=85;
  uint16_t Ymax=900;
#endif
//============================================================================
// This code can synchronize between two identical demonstration setups.
// Connect pin 5 of the master to pin 5 of the slave, and pin 6 of the master
// to pin 6 of the slave. Upload to both, then reset both at the same time.
#define SYNC_NONE   (0)
#define SYNC_MASTER (1)
#define SYNC_SLAVE  (2)

#if(0) //1 = sync slides for video
  #if(display == CFAF240320V020T)
    // Normal TN TFT
    #define SYNC SYNC_SLAVE
  #endif
  #if(display == CFAF240320W020T)
    //IPS TFT
    #define SYNC SYNC_MASTER
  #endif
  #if(display == CFAF240320X020T)
    //O-Film TFT
    #define SYNC SYNC_SLAVE
  #endif
#else
  #define SYNC SYNC_NONE
#endif
//
//============================================================================
// LCD SPI & control lines
//   ARD      | Port | LCD
// -----------+------+--------------------------------------------------------
//  #5/D5     |  PD5 | SYNC_PIN_SLAVE  (optional -- generally not used)
//  #6/D6     |  PD6 | SYNC_PIN_MASTER (optional -- generally not used)
//  #7/D7     |  PD7 | SD_CS    
//  #8/D8     |  PB0 | LCD_RS
//  #9/D9     |  PB1 | LCD_RESET
// #10/D10    |  PB2 | LCD_CS_NOT (or SPI SS)
// #11/D11    |  PB3 | LCD_MOSI   (hardware SPI)
// #12/D12    |  PB4 | not used   (would be MISO)
// #13/D13    |  PB5 | LCD_SCK    (hardware SPI)
// #23/D14/A0 |  PC0 | Touch XL   (only used on TS modules)
// #24/D15/A1 |  PC1 | Touch XR   (only used on TS modules)
// #25/D16/A2 |  PC2 | Touch YD   (only used on TS modules)
// #26/D17/A3 |  PC3 | Touch YU   (only used on TS modules)
//============================================================================
//
#define RS_PIN    8
#define RESET_PIN 9
#define CS_PIN    10
#define SCK_PIN   13
#define MOSI_PIN  11

#define CLR_RS    ( digitalWrite(RS_PIN, 0) )
#define SET_RS    ( digitalWrite(RS_PIN, 1) )
#define CLR_RESET ( digitalWrite(RESET_PIN, 0) )
#define SET_RESET ( digitalWrite(RESET_PIN, 1) )
#define CLR_CS    ( digitalWrite(CS_PIN, 0) )
#define SET_CS    ( digitalWrite(CS_PIN, 1) )

#define CLR_MOSI  ( digitalWrite(MOSI_PIN, 0) )
#define SET_MOSI  ( digitalWrite(MOSI_PIN, 1) )
#define CLR_SCK   ( digitalWrite(SCK_PIN, 0) )
#define SET_SCK   ( digitalWrite(SCK_PIN, 1) )

#define TS_XL (14)
#define TS_XR (15)
#define TS_YD (16)
#define TS_YU (17)
#define SYNC_PIN_MASTER (6)
#define SYNC_PIN_SLAVE (5)
//============================================================================
void SPI_sendCommand(uint8_t command)
  {
  // Select the LCD's command register
  CLR_RS;
  // Select the LCD controller
  CLR_CS;
  //Send the command via SPI:
  SPI.transfer(command);
  // Deselect the LCD controller
  SET_CS;
  }
//----------------------------------------------------------------------------
void SPI_sendData(uint8_t data)
  {
  // Select the LCD's data register
  SET_RS;
  // Select the LCD controller
  CLR_CS;
  //Send the data via SPI:
  SPI.transfer(data);
  // Deselect the LCD controller
  SET_CS;
  }
//----------------------------------------------------------------------------
// Defines for the ST7789 registers.
// ref: https://www.crystalfontz.com/controllers/Sitronix/ST7789V/
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
//----------------------------------------------------------------------------
void Initialize_LCD(void)
  {
  //Reset the LCD controller
  CLR_RESET;
  delay(1);//10µS min
  SET_RESET;
  delay(150);//120mS max

  // SLPOUT (11h): Sleep Out ("Sleep Out"  is chingrish for "wake")
  // The DC/DC converter is enabled, Internal display oscillator
  // is started, and panel scanning is started.
  SPI_sendCommand(ST7789_11_SLPOUT); 
  delay(120);      //Delay 120ms 

  // MADCTL (36h): Memory Data Access Control
  // Set the RGB vs BGR order to match a windows 24-bit BMP 
  SPI_sendCommand(ST7789_36_MADCTL);
  SPI_sendData(0x08);// YXVL RH--
                     // |||| ||||-- Unused: 0
                     // |||| ||---- MH: Horizontal Refresh Order
                     // |||| |        0 = left to right
                     // |||| |        1 = right to left
                     // |||| |----- RGB: RGB vs BGR Order
                     // ||||          0 = RGB color filter panel
                     // ||||          1 = BGR color filter panel
                     // ||||------- ML: Vertical Refresh Order
                     // |||           0 = top to bottom
                     // |||           1 = bottom to top
                     // |||-------- MV: Row / Column Exchange
                     // ||--------- MX: Column Address Order  <<<<<
                     // |---------- MY: Row Address Order

  // COLMOD (3Ah): Interface Pixel Format
  // * This command is used to define the format of RGB picture
  //   data, which is to be transferred via the MCU interface.
  SPI_sendCommand(ST7789_3A_COLMOD);
  SPI_sendData(0x06);// Default: 0x06 => 18-bit/pixel
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
  SPI_sendCommand(ST7789_B2_PORCTRL);
  SPI_sendData(0x0C);   //BPA[6:0]: Back porch setting in normal mode. The minimum setting is 0x01.
  SPI_sendData(0x0C);   //FPA[6:0]: Front porch setting in normal mode. The minimum setting is 0x01.
  SPI_sendData(0x00);   //Disable(0) / Enable (1) separate porch control
  SPI_sendData(0x33);   //(high nibble) BPB[3:0]: Back porch setting in idle mode. The minimum setting is 0x01.
                        //(low nibble)  FPB[3:0]: Front porch setting in idle mode. The minimum setting is 0x01.
  SPI_sendData(0x33);   //(high nibble) BPB[3:0]: Back porch setting in partial mode. The minimum setting is 0x01.
                        //(low nibble)  FPC[3:0]: Front porch setting in partial mode. The minimum setting is 0x01.

  // GCTRL (B7h): Gate Control
  SPI_sendCommand(ST7789_B7_GCTRL);
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(0x34);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(0x35);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(0x35);
#endif
    //(high nibble) VGHS[2:0]: VGH Setting.
    // 0x0- => 12.20v
    // 0x1- => 12.54v
    // 0x2- => 12.89v
    // 0x3- => 13.26v <<<<<
    // 0x4- => 13.65v
    // 0x5- => 14.06v
    // 0x6- => 14.50v
    // 0x7- => 14.97v
    //(low nibble) VGLS[2:0]: VGL Setting.
    // 0x-0 =>  -7.16v
    // 0x-1 =>  -7.67v
    // 0x-2 =>  -8.23v
    // 0x-3 =>  -8.87v
    // 0x-4 =>  -9.60v
    // 0x-5 => -10.43v <<<<<
    // 0x-6 => -11.38v
    // 0x-7 => -12.50v

  // VCOMS (BBh): VCOM Setting
  SPI_sendCommand(ST7789_BB_VCOMS);
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(0x1C);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(0x2B);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(0x35);
#endif
    // VCOMS[5:0]
    // 0x00 => 0.100v
    // 0x01 => 0.125v
    // 0x02 => 0.150v
    // 0x03 => 0.175v
    // 0x04 => 0.200v
    // 0x05 => 0.225v
    // 0x06 => 0.250v
    // 0x07 => 0.275v
    // 0x08 => 0.300v
    // 0x09 => 0.325v
    // 0x0A => 0.350v
    // 0x0B => 0.375v
    // 0x0C => 0.400v
    // 0x0D => 0.425v
    // 0x0E => 0.450v
    // 0x0F => 0.475v
    // 0x10 => 0.500v
    // 0x11 => 0.525v
    // 0x12 => 0.550v
    // 0x13 => 0.575v
    // 0x14 => 0.600v
    // 0x15 => 0.625v
    // 0x16 => 0.650v
    // 0x17 => 0.675v
    // 0x18 => 0.700v
    // 0x19 => 0.725v
    // 0x1A => 0.750v
    // 0x1B => 0.775v
    // 0x1C => 0.800v <<<< V
    // 0x1D => 0.825v
    // 0x1E => 0.850v
    // 0x1F => 0.875v
    // 0x20 => 0.900v
    // 0x21 => 0.925v
    // 0x22 => 0.950v
    // 0x23 => 0.975v
    // 0x24 => 1.000v
    // 0x25 => 1.025v
    // 0x26 => 1.050v
    // 0x27 => 1.075v
    // 0x28 => 1.100v
    // 0x29 => 1.125v
    // 0x2A => 1.150v
    // 0x2B => 1.175v <<<< W
    // 0x2C => 1.200v
    // 0x2D => 1.225v
    // 0x2E => 1.250v
    // 0x2F => 1.275v
    // 0x30 => 1.300v
    // 0x31 => 1.325v
    // 0x32 => 1.350v
    // 0x33 => 1.375v
    // 0x34 => 1.400v
    // 0x35 => 1.425v <<<< X
    // 0x36 => 1.450v
    // 0x37 => 1.475v
    // 0x38 => 1.500v
    // 0x39 => 1.525v
    // 0x3A => 1.550v
    // 0x3B => 1.575v
    // 0x3C => 1.600v
    // 0x3D => 1.625v
    // 0x3E => 1.650v
    // 0x3F => 1.675v  

  // LCMCTRL (C0h): LCM Control
  SPI_sendCommand(ST7789_C0_LCMCTRL);     
  SPI_sendData(0x2C);   
    // 0010 1100
    // |||| ||||--  GS: Gate scan inversion enable: 1: enable, 0: disable.
    // |||| |||---  XMX: XOR MX setting in command 36h.
    // |||| ||---- *XMV: XOR MV setting in command 36h.
    // |||| |----- *XMH: XOR RGB setting in command 36h, in RGB interface
    // ||||             without RAM mode can support column address order.
    // ||||-------  XREV: XOR inverse setting in command 21h.
    // |||-------- *XBGR: XOR RGB setting in command 36h.
    // ||---------  XMY: XOR MY setting in command 36h.
    // |----------  Unused: 0

  // VDVVRHEN (C2h): VDV and VRH Command Enable
  SPI_sendCommand(ST7789_C2_VDVVRHEN);     
  SPI_sendData(0x01); // CMDEN=”0”: VDV and VRH register value comes from NVM.
                      // CMDEN=”1”, VDV and VRH register value comes from command write.
  SPI_sendData(0xFF);   

  // VRHS (C3h): VRH Set
  SPI_sendCommand(ST7789_C3_VRHS);
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(0x0B);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(0x20);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(0x11);
#endif
    // 0x00 => VAP(GVDD)(V) = +3.55 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) = -3.55 + (vcom + vcom_offset - 0.5vdv)
    // 0x01 => VAP(GVDD)(V) = +3.60 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) = -3.60 + (vcom + vcom_offset - 0.5vdv)
    // 0x02 => VAP(GVDD)(V) = +3.65 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) = -3.65 + (vcom + vcom_offset - 0.5vdv)
    // 0x03 => VAP(GVDD)(V) = +3.70 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 3.70 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x04 => VAP(GVDD)(V) = +3.75 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 3.75 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x05 => VAP(GVDD)(V) = +3.80 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 3.80 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x06 => VAP(GVDD)(V) = +3.85 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 3.85 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x07 => VAP(GVDD)(V) = +3.90 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 3.90 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x08 => VAP(GVDD)(V) = +3.95 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 3.95 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x09 => VAP(GVDD)(V) = +4.00 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.00 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x0A => VAP(GVDD)(V) = +4.05 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.05 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x0B => VAP(GVDD)(V) = +4.10 + (vcom + vcom_offset + 0.5vdv) <<<< V
    //         VAN(GVCL)(V) =- 4.10 + (vcom + vcom_offset-+ 0.5vdv) <<<< V
    // 0x0C => VAP(GVDD)(V) = +4.15 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.15 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x0D => VAP(GVDD)(V) = +4.20 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.20 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x0E => VAP(GVDD)(V) = +4.25 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.25 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x0F => VAP(GVDD)(V) = +4.30 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.30 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x10 => VAP(GVDD)(V) = +4.35 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.35 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x11 => VAP(GVDD)(V) = +4.40 + (vcom + vcom_offset + 0.5vdv) <<<< X
    //         VAN(GVCL)(V) =- 4.40 + (vcom + vcom_offset-+ 0.5vdv) <<<< X
    // 0x12 => VAP(GVDD)(V) = +4.45 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.45 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x13 => VAP(GVDD)(V) = +4.50 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.50 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x14 => VAP(GVDD)(V) = +4.55 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.55 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x15 => VAP(GVDD)(V) = +4.60 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.60 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x16 => VAP(GVDD)(V) = +4.65 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.65 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x17 => VAP(GVDD)(V) = +4.70 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.70 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x18 => VAP(GVDD)(V) = +4.75 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.75 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x19 => VAP(GVDD)(V) = +4.80 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.80 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x1A => VAP(GVDD)(V) = +4.85 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.85 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x1B => VAP(GVDD)(V) = +4.90 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.90 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x1C => VAP(GVDD)(V) = +4.95 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 4.95 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x1D => VAP(GVDD)(V) = +5.00 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.00 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x1E => VAP(GVDD)(V) = +5.05 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.05 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x1F => VAP(GVDD)(V) = +5.10 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.10 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x20 => VAP(GVDD)(V) = +5.15 + (vcom + vcom_offset + 0.5vdv) <<<< W
    //         VAN(GVCL)(V) =- 5.15 + (vcom + vcom_offset-+ 0.5vdv) <<<< W
    // 0x21 => VAP(GVDD)(V) = +5.20 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.20 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x22 => VAP(GVDD)(V) = +5.25 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.25 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x23 => VAP(GVDD)(V) = +5.30 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.30 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x24 => VAP(GVDD)(V) = +5.35 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.35 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x25 => VAP(GVDD)(V) = +5.40 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.40 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x26 => VAP(GVDD)(V) = +5.45 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.45 + (vcom + vcom_offset-+ 0.5vdv)
    // 0x27 => VAP(GVDD)(V) = +5.50 + (vcom + vcom_offset + 0.5vdv)
    //         VAN(GVCL)(V) =- 5.50 + (vcom + vcom_offset-+ 0.5vdv)
  // 0x28~0x3Fh => Reserved

  //VDVS (C4h): VDV Set
  SPI_sendCommand(ST7789_C4_VDVSET);
  SPI_sendData(0x20);
    // 0x00 => -0.800
    // 0x01 => -0.775
    // 0x02 => -0.750
    // 0x03 => -0.725
    // 0x04 => -0.700
    // 0x05 => -0.675
    // 0x06 => -0.650
    // 0x07 => -0.625
    // 0x08 => -0.600
    // 0x09 => -0.575
    // 0x0A => -0.550
    // 0x0B => -0.525
    // 0x0C => -0.500
    // 0x0D => -0.475
    // 0x0E => -0.450
    // 0x0F => -0.425
    // 0x10 => -0.400
    // 0x11 => -0.375
    // 0x12 => -0.350
    // 0x13 => -0.325
    // 0x14 => -0.300
    // 0x15 => -0.275
    // 0x16 => -0.250
    // 0x17 => -0.225
    // 0x18 => -0.200
    // 0x19 => -0.175
    // 0x1A => -0.150
    // 0x1B => -0.125
    // 0x1C => -0.100
    // 0x1D => -0.075
    // 0x1E => -0.050
    // 0x1F => -0.025
    // 0x20 => +0.000 <<<<<
    // 0x21 => +0.025
    // 0x22 => +0.050
    // 0x23 => +0.075
    // 0x24 => +0.100
    // 0x25 => +0.125
    // 0x26 => +0.150
    // 0x27 => +0.175
    // 0x28 => +0.200
    // 0x29 => +0.225
    // 0x2A => +0.250
    // 0x2B => +0.275
    // 0x2C => +0.300
    // 0x2D => +0.325
    // 0x2E => +0.350
    // 0x2F => +0.375
    // 0x30 => +0.400
    // 0x31 => +0.425
    // 0x32 => +0.450
    // 0x33 => +0.475
    // 0x34 => +0.500
    // 0x35 => +0.525
    // 0x36 => +0.550
    // 0x37 => +0.575
    // 0x38 => +0.600
    // 0x39 => +0.625
    // 0x3A => +0.650
    // 0x3B => +0.675
    // 0x3C => +0.700
    // 0x3D => +0.725
    // 0x3E => +0.750
    // 0x3F => +0.775
  
  //FRCTRL2 (C6h): Frame Rate Control in Normal Mode
  SPI_sendCommand(ST7789_C6_FRCTR2);
  SPI_sendData(0x0F);
    // 0000 1111
    // ||||-||||--  RTNA[4:0]
    // |||--------  NLA[2:0] : Inversion selection in normal mode
    //                000 = dot inversion
    //                111 = column inversion
    // RTNA[4:0] => FR in normal mode (Hz)
    // 0x00 => 119 Hz
    // 0x01 => 111 Hz
    // 0x02 => 105 Hz
    // 0x03 =>  99 Hz
    // 0x04 =>  94 Hz
    // 0x05 =>  90 Hz
    // 0x06 =>  86 Hz
    // 0x07 =>  82 Hz
    // 0x08 =>  78 Hz
    // 0x09 =>  75 Hz
    // 0x0A =>  72 Hz
    // 0x0B =>  69 Hz
    // 0x0C =>  67 Hz
    // 0x0D =>  64 Hz
    // 0x0E =>  62 Hz
    // 0x0F =>  60 Hz <<<<<
    // 0x10 =>  58 Hz
    // 0x11 =>  57 Hz
    // 0x12 =>  55 Hz
    // 0x13 =>  53 Hz
    // 0x14 =>  52 Hz
    // 0x15 =>  50 Hz
    // 0x16 =>  49 Hz
    // 0x17 =>  48 Hz
    // 0x18 =>  46 Hz
    // 0x19 =>  45 Hz
    // 0x1A =>  44 Hz
    // 0x1B =>  43 Hz
    // 0x1C =>  42 Hz
    // 0x1D =>  41 Hz
    // 0x1E =>  40 Hz
    // 0x1F =>  39 Hz

  // PWCTRL1 (D0h): Power Control 1
  SPI_sendCommand(ST7789_D0_PWCTRL1);
  SPI_sendData(0xA4);   //Fixed vector
  SPI_sendData(0xA1);   //AVDD=6.8V, AVCL=-4.8V, VDS=2.3V
    // DDCC --VV
    // |||| ||||--  VDS[1:0]:
    // ||||           00 = 2.19v
    // ||||           01 = 2.30v <<<<<
    // ||||           10 = 2.40v
    // ||||           11 = 2.51v
    // ||||-------  AVCL[1:0]:
    // ||             00 = -4.4v
    // ||             01 = -4.6v
    // ||             10 = -4.8v <<<<<
    // ||             11 = -5.0v
    // ||---------  AVDD[1:0]:
    //                00 = 6.4v
    //                01 = 6.6v
    //                10 = 6.8v <<<<<
    //                11 = Reserved

  
  // PVGAMCTRL (E0h): Positive Voltage Gamma Control
  SPI_sendCommand(ST7789_E0_PVGAMCTRL);     
  // Pile of magic numbers :-(
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(0xD0);   
  SPI_sendData(0x00);   
  SPI_sendData(0x03);   
  SPI_sendData(0x09);   
  SPI_sendData(0x09);   
  SPI_sendData(0x17);   
  SPI_sendData(0x2A);   
  SPI_sendData(0x44);   
  SPI_sendData(0x3C);   
  SPI_sendData(0x2B);   
  SPI_sendData(0x17);   
  SPI_sendData(0x15);   
  SPI_sendData(0x10);   
  SPI_sendData(0x13);  
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(0xD0);
  SPI_sendData(0xCA);
  SPI_sendData(0x0E);
  SPI_sendData(0x08);
  SPI_sendData(0x09);
  SPI_sendData(0x07);
  SPI_sendData(0x2D);
  SPI_sendData(0x3B);
  SPI_sendData(0x3D);
  SPI_sendData(0x34);
  SPI_sendData(0x0A);
  SPI_sendData(0x0A);
  SPI_sendData(0x1B);
  SPI_sendData(0x28);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(0xd0);
  SPI_sendData(0x00);
  SPI_sendData(0x06);
  SPI_sendData(0x09);
  SPI_sendData(0x0b);
  SPI_sendData(0x2a);
  SPI_sendData(0x3c);
  SPI_sendData(0x55);
  SPI_sendData(0x4b);
  SPI_sendData(0x08);
  SPI_sendData(0x16);
  SPI_sendData(0x14);
  SPI_sendData(0x19);
  SPI_sendData(0x20);
#endif

  // NVGAMCTRL (E1h): Negative Voltage Gamma Control
  SPI_sendCommand(ST7789_E1_NVGAMCTRL);     
  // Pile of magic numbers :-(
#if(display == CFAF240320V020T)
  // Normal TN TFT
  SPI_sendData(0xD0);   
  SPI_sendData(0x00);   
  SPI_sendData(0x02);   
  SPI_sendData(0x08);   
  SPI_sendData(0x08);   
  SPI_sendData(0x27);   
  SPI_sendData(0x26);   
  SPI_sendData(0x54);   
  SPI_sendData(0x3B);   
  SPI_sendData(0x3B);   
  SPI_sendData(0x16);   
  SPI_sendData(0x15);   
  SPI_sendData(0x0F);   
  SPI_sendData(0x13); 
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  SPI_sendData(0xD0);
  SPI_sendData(0xCA);
  SPI_sendData(0x0F);
  SPI_sendData(0x08);
  SPI_sendData(0x08);
  SPI_sendData(0x07);
  SPI_sendData(0x2E);
  SPI_sendData(0x5C);
  SPI_sendData(0x40);
  SPI_sendData(0x34);
  SPI_sendData(0x09);
  SPI_sendData(0x0B);
  SPI_sendData(0x1B);
  SPI_sendData(0x28);
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  SPI_sendData(0xd0);
  SPI_sendData(0x00);
  SPI_sendData(0x06);
  SPI_sendData(0x09);
  SPI_sendData(0x0b);
  SPI_sendData(0x29);
  SPI_sendData(0x36);
  SPI_sendData(0x54);
  SPI_sendData(0x4b);
  SPI_sendData(0x0d);
  SPI_sendData(0x16);
  SPI_sendData(0x14);
  SPI_sendData(0x21);
  SPI_sendData(0x20);
#endif
 

#if(display == CFAF240320V020T)
  // Normal TN TFT
  // INVOFF (20h): Display Inversion Off (correct for V)
  SPI_sendData(ST7789_20_INVOFF);
#endif
#if(display == CFAF240320W020T)
  //IPS TFT
  // INVON (21h): Display Inversion On (correct for W)
  SPI_sendCommand(ST7789_21_INVON); 
#endif
#if(display == CFAF240320X020T)
  //O-Film TFT
  // INVOFF (20h): Display Inversion Off (correct for X)
  SPI_sendData(ST7789_20_INVOFF);
#endif

  // CASET (2Ah): Column Address Set
  SPI_sendCommand(ST7789_2A_CASET);
  SPI_sendData(0x00); //Start MSB Start = 0
  SPI_sendData(0x00); //Start LSB
  SPI_sendData(0x00); //End MSB End = 249
  SPI_sendData(0xEF); //End LSB

  // RASET (2Bh): Row Address Set
  SPI_sendCommand(ST7789_2B_RASET);
  SPI_sendData(0x00); //Start MSB Start = 0
  SPI_sendData(0x00); //Start LSB
  SPI_sendData(0x01); //End MSB End = 319
  SPI_sendData(0x3F); //End LSB

  // DISPON (29h): Display On
  SPI_sendCommand(ST7789_29_DISPON);
  delay(1);
  }
//============================================================================
void Set_LCD_for_write_at_X_Y(uint16_t x, uint16_t y)
  {
  //CASET (2Ah): Column Address Set
  // * The value of XS [15:0] and XE [15:0] are referred when RAMWR
  //   command comes.
  // * Each value represents one column line in the Frame Memory.
  // * XS [15:0] always must be equal to or less than XE [15:0]
  SPI_sendCommand(ST7789_2A_CASET); //Column address set
  //Write the parameters for the "column address set" command
  SPI_sendData(x>>8);     //Start MSB = XS[15:8]
  SPI_sendData(x&0x00FF); //Start LSB = XS[ 7:0]
  SPI_sendData(0);        //End MSB   = XE[15:8] 240-1
  SPI_sendData(240);      //End LSB   = XE[ 7:0]
  //Write the "row address set" command to the LCD
  //RASET (2Bh): Row Address Set
  // * The value of YS [15:0] and YE [15:0] are referred when RAMWR
  //   command comes.
  // * Each value represents one row line in the Frame Memory.
  // * YS [15:0] always must be equal to or less than YE [15:0]
  SPI_sendCommand(ST7789_2B_RASET); //Row address set
  //Write the parameters for the "row address set" command
  //Use 1st quadrant coordinates: 0,0 is lower left, 239,319 is upper right.
  y=319-y;
  SPI_sendData(y>>8);     //Start MSB = YS[15:8]
  SPI_sendData(y&0x00FF); //Start LSB = YS[ 7:0]
  SPI_sendData(0x01);     //End MSB   = YE[15:8] 320-1
  SPI_sendData(0x3F);     //End LSB   = YE[ 7:0]
  //Write the "write data" command to the LCD
  //RAMWR (2Ch): Memory Write
  SPI_sendCommand(ST7789_2C_RAMWR); //write data
  }
//============================================================================
#if(0) //simple
void Fill_LCD(uint8_t R, uint8_t G, uint8_t B)
  {
  uint32_t
    i;
  Set_LCD_for_write_at_X_Y(0, 319);

  //Fill display with a given RGB value
  for (i = 0; i < (320UL * 240UL); i++)
    {
    SPI_sendData(B); //Blue
    SPI_sendData(G); //Green
    SPI_sendData(R); //Red
    }
  }
#else //faster, bigger (6 bytes)
void Fill_LCD(uint8_t R, uint8_t G, uint8_t B)
  {
  uint32_t
    i;
  Set_LCD_for_write_at_X_Y(0, 319);

  // Select the LCD controller
  CLR_CS;
  // Select the LCD's data register
  SET_RS;

  //Fill display with a given RGB value
  for (i = 0; i < (320UL * 240UL); i++)
    {
    SPI.transfer(B); //Blue
    SPI.transfer(G); //Green
    SPI.transfer(R); //Red
    }
  // Deselect the LCD controller
  SET_CS;    
  }
#endif
//============================================================================
#if(0) //simple
void Put_Pixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
  {
  Set_LCD_for_write_at_X_Y(x, y);
  //Write the single pixel's worth of data
  SPI_sendData(B); //Blue
  SPI_sendData(G); //Green
  SPI_sendData(R); //Red
  }
#else //faster, bigger (78 bytes)
void Put_Pixel(uint16_t x, uint16_t y, uint8_t R, uint8_t G, uint8_t B)
  {
  // Select the LCD controller
  CLR_CS;
  //CASET (2Ah): Column Address Set
  // * The value of XS [15:0] and XE [15:0] are referred when RAMWR
  //   command comes.
  // * Each value represents one column line in the Frame Memory.
  // * XS [15:0] always must be equal to or less than XE [15:0]
  // Select the LCD's command register
  CLR_RS;  
  SPI.transfer(ST7789_2A_CASET); //Column address set
  // Select the LCD's data register
  SET_RS;
  //Write the parameters for the "column address set" command
  SPI.transfer(x>>8);     //Start MSB = XS[15:8]
  SPI.transfer(x&0x00FF); //Start LSB = XS[ 7:0]
  SPI.transfer(0);        //End MSB   = XE[15:8] 240-1
  SPI.transfer(240);      //End LSB   = XE[ 7:0]
  //Write the "row address set" command to the LCD
  //RASET (2Bh): Row Address Set
  // * The value of YS [15:0] and YE [15:0] are referred when RAMWR
  //   command comes.
  // * Each value represents one row line in the Frame Memory.
  // * YS [15:0] always must be equal to or less than YE [15:0]
  // Select the LCD's command register
  CLR_RS;  
  SPI.transfer(ST7789_2B_RASET); //Row address set
  // Select the LCD's data register
  SET_RS;
  //Use 1st quadrant coordinates: 0,0 is lower left, 239,319 is upper right.
  y=319-y;
  //Write the parameters for the "row address set" command
  SPI.transfer(y>>8);     //Start MSB = YS[15:8]
  SPI.transfer(y&0x00FF); //Start LSB = YS[ 7:0]
  SPI.transfer(0x01);     //End MSB   = YE[15:8] 320-1
  SPI.transfer(0x3F);     //End LSB   = YE[ 7:0]
  //Write the "write data" command to the LCD
  //RAMWR (2Ch): Memory Write
  // Select the LCD's command register
  CLR_RS;  
  SPI.transfer(ST7789_2C_RAMWR); //write data
  //Write the single pixel's worth of data
  // Select the LCD's data register
  SET_RS;
  SPI.transfer(B); //Blue
  SPI.transfer(G); //Green
  SPI.transfer(R); //Red
  // Deselect the LCD controller
  SET_CS;
  }
#endif
//============================================================================
// From: http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
void LCD_Circle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t R, uint16_t G, uint16_t B)
  {
  uint16_t x = radius;
  uint16_t y = 0;
  int16_t radiusError = 1 - (int16_t) x;

  while (x >= y)
    {
    //11 O'Clock
    Put_Pixel(x0 - y, y0 + x, R, G, B);
    //1 O'Clock
    Put_Pixel(x0 + y, y0 + x, R, G, B);
    //10 O'Clock
    Put_Pixel(x0 - x, y0 + y, R, G, B);
    //2 O'Clock
    Put_Pixel(x0 + x, y0 + y, R, G, B);
    //8 O'Clock
    Put_Pixel(x0 - x, y0 - y, R, G, B);
    //4 O'Clock
    Put_Pixel(x0 + x, y0 - y, R, G, B);
    //7 O'Clock
    Put_Pixel(x0 - y, y0 - x, R, G, B);
    //5 O'Clock
    Put_Pixel(x0 + y, y0 - x, R, G, B);

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
//============================================================================
#define mSwap(a,b,t)\
  {\
  t=a;\
  a=b;\
  b=t;\
  }\
//----------------------------------------------------------------------------
void Fast_Horizontal_Line(uint16_t x0, uint16_t y, uint16_t x1,
                          uint8_t r, uint8_t g, uint8_t b)
  {
  uint16_t
    temp;
  if(x1 < x0)
    mSwap(x0, x1, temp);
  Set_LCD_for_write_at_X_Y(x0, y);
  while(x0 <= x1)
    {
    //Write the single pixel's worth of data
    SPI_sendData(r); //Blue
    SPI_sendData(g); //Green
    SPI_sendData(b); //Red
    x0++;
    }
  }
//============================================================================
// From: http://rosettacode.org/wiki/Bitmap/Bresenham's_line_algorithm#C
void LCD_Line(uint16_t x0, uint16_t y0,
              uint16_t x1, uint16_t y1,
              uint8_t r, uint8_t g, uint8_t b)
  {
  int16_t
    dx;
  int16_t
    sx;
  int16_t
    dy;
  int16_t
    sy;
  int16_t
    err;
  int16_t
   e2;

  //General case
  if (y0 != y1)
    {
    dx = abs((int16_t )x1 - (int16_t )x0);
    sx = x0 < x1 ? 1 : -1;
    dy = abs((int16_t )y1 - (int16_t )y0);
    sy = y0 < y1 ? 1 : -1;
    err = (dx > dy ? dx : -dy) / 2;

    for (;;)
      {
      Put_Pixel(x0, y0, r,g,b);
      if ((x0 == x1) && (y0 == y1))
        break;
      e2 = err;
      if (e2 > -dx)
        {
        err -= dy;
        x0 = (uint16_t)((int16_t) x0 + sx);
        }
      if (e2 < dy)
        {
        err += dx;
        y0 = (uint16_t)((int16_t) y0 + sy);
        }
      }
    }
  else
    {
    //Optimized for LCD
    Fast_Horizontal_Line(x0, y0, x1,r,g,b);
    }
  } 

/*
//============================================================================
// This function transfers data, in one stream. Slightly
// optimized to do index operations during SPI transfers.
void SPI_send_pixels(uint8_t byte_count, uint8_t *data_ptr)
  {
  uint8_t
    subpixel;
    
  // Select the LCD's data register
  SET_RS;
  // Select the LCD controller
  CLR_CS;

  //Load the first byte
  subpixel=*data_ptr;
  
  while(byte_count)
    {
    //Send the byte out.
    SPDR = subpixel;
    //do something that can happen while transmitting
    data_ptr++; //point to next byte
    //Load the next byte
    subpixel=*data_ptr;
    //count this byte
    byte_count--;
    //Now that we have done all we can do, wait for the transfer to finish.
    while (!(SPSR & _BV(SPIF))) ;
    }
  // Deselect the LCD controller
  SET_CS;
  }
*/
  
//----------------------------------------------------------------------------
void show_BMPs_in_root(void)
  {
  File
    root_dir;
  root_dir = SD.open("/");
  if(0 == root_dir)
    {
    Serial.println("show_BMPs_in_root: Can't open \"root\"");
    }
  File
    bmp_file;

  while(1)
    {
#if(SYNC == SYNC_MASTER)
    //Signal that we are ready and waiting
    digitalWrite(SYNC_PIN_MASTER,HIGH);
    //Wait for the slave to be ready
    while(0 == digitalRead(SYNC_PIN_SLAVE));
#endif
#if(SYNC == SYNC_SLAVE)
    //Wait till the master is ready
    while(0 == digitalRead(SYNC_PIN_MASTER));
    //Signal what we are ready
    digitalWrite(SYNC_PIN_SLAVE,HIGH);
#endif

    bmp_file = root_dir.openNextFile();
    if (0 == bmp_file)
      {
      // no more files, break out of while()
      // root_dir will be closed below.
      break;
      }
    //Skip directories (what about volume name?)
    if(0 == bmp_file.isDirectory())
      {
      //The file name must include ".BMP"
      if(0 != strstr(bmp_file.name(),".BMP"))
        {
        //The BMP must be exactly 230456 long
        //(this is correct for 240x320, 24-bit)
        if(230456 == bmp_file.size())
          {
          //Jump over BMP header. BMP must be 240x320 24-bit
          bmp_file.seek(54);
    
          //Since we are limited in memory, break the line up from
          // 240*3 = 720 bytes into three chunks of 80 pixels
          // each 80*3 = 240 bytes.
          //Making this static speeds it up slightly (10ms)
          //Reduces flash size by 114 bytes, and uses 240 bytes.
          static uint8_t
            third_of_a_line[80*3];
          for(uint16_t line=0;line<320;line++)
            {
            //Set the LCD to the left of this line. BMPs store data
            //lowest line first -- bottom up.
            Set_LCD_for_write_at_X_Y(0,line);
            for(uint8_t line_section=0;line_section<3;line_section++)
              {
              //Get a third of the line
              bmp_file.read(third_of_a_line,80*3);
              //Now write this third to the TFT, doing the BGR -> RGB
              //color fixup interlaced with the SPI transfers.
              //SPI_send_pixels(80*3,third_of_a_line);
              }
            }
           }
         }
       }
    //Release the BMP file handle
    bmp_file.close();

#if(SYNC == SYNC_MASTER)
    //Signal that we are done and waiting
    digitalWrite(SYNC_PIN_MASTER,LOW);
    //Wait for the slave to be ready
    while(0 != digitalRead(SYNC_PIN_SLAVE));
#endif
#if(SYNC == SYNC_SLAVE)
    //Wait till the master is done
    while(0 != digitalRead(SYNC_PIN_MASTER));
    //Wait for a bit
     delay(1000);
    //Signal what we are done
    digitalWrite(SYNC_PIN_SLAVE,LOW);
#endif
#if(SYNC == SYNC_NONE)
    delay(1000);
#endif
    }
  //Release the root directory file handle
  root_dir.close();
  }
//============================================================================
#if TOUCH_ENABLED
uint8_t Read_Touch_Screen(uint16_t *x, uint16_t *y)
  {
  //See if there is a touch.
  //Let YU float, make YD tug high, drive X1 and X2 low.
  //Read Y1, if it is near 5v (~1024), then the screen is not
  //touched. If it is near ground (~50) then the screen is
  //touched.
  uint16_t
    touched;
  pinMode(TS_YU,INPUT);
  digitalWrite(TS_YU,HIGH);
  pinMode(TS_YD,INPUT_PULLUP);  
  digitalWrite(TS_YD,HIGH);
  pinMode(TS_XL,OUTPUT);
  digitalWrite(TS_XL,LOW);
  pinMode(TS_XR,OUTPUT);
  digitalWrite(TS_XR,LOW);
  touched = analogRead(TS_YU);

  //Idle YD as an input
  pinMode(TS_YD,INPUT);  
  if(touched < 512)
    {
    //We are touched.
    uint32_t
      X;
    uint32_t
      Y;
    //Read X. Set a gradient from 0v to 5v on X, then
    //read the Y line to get the X contact point.
    //pinMode(TS_YU,INPUT);    //Already set
    //pinMode(TS_YD,INPUT);    //Already set
    //pinMode(TS_XL,OUTPUT);   //Already set
    digitalWrite(TS_XR,HIGH);
    //pinMode(TS_XR,OUTPUT);   //Already set
    //digitalWrite(TS_XL,LOW); //Already set
    X = analogRead(TS_YD);     //Could use YU

    //Read Y. Set a gradient from 0v to 5v on Y, then
    //read the X line to get the Y contact point.
    pinMode(TS_XL,INPUT);
    pinMode(TS_XR,INPUT);
    pinMode(TS_YU,OUTPUT);
    digitalWrite(TS_YU,HIGH);
    pinMode(TS_YD,OUTPUT);
    digitalWrite(TS_YD,LOW);
    Y = analogRead(TS_XL);     //Could use XR
    
    //Idle the Y pins
    pinMode(TS_YU,INPUT);
    pinMode(TS_YD,INPUT);

    //Calculate the pixel values, store in the user's pointers.
    *x=((X-(uint32_t)Xmin)*240)/((uint32_t)Xmax-(uint32_t)Xmin);
    *y=((Y-(uint32_t)Ymin)*320)/((uint32_t)Ymax-(uint32_t)Ymin);
   
    //Return touched flag.
    return(1);
    }
  else
    {    
    //Not touched. Idle the pins that were set to output
    //to detect the touch.
    pinMode(TS_XL,INPUT);
    pinMode(TS_XR,INPUT);
    return(0);
    }
  } 
#endif
//============================================================================
void setup()
  {
  //==========================================================================
  // LCD SPI & control lines
  //   ARD      | Port | LCD
  // -----------+------+------------------------------------------------------
  //  #5/D5     |  PD5 | SYNC_PIN_SLAVE  (optional -- generally not used)
  //  #6/D6     |  PD6 | SYNC_PIN_MASTER (optional -- generally not used)
  //  #7/D7     |  PD7 | SD_CS    
  //  #8/D8     |  PB0 | LCD_RS
  //  #9/D9     |  PB1 | LCD_RESET
  // #10/D10    |  PB2 | LCD_CS_NOT (or SPI SS)
  // #11/D11    |  PB3 | LCD_MOSI   (hardware SPI)
  // #12/D12    |  PB4 | not used   (would be MISO)
  // #13/D13    |  PB5 | LCD_SCK    (hardware SPI)
  // #23/D14/A0 |  PC0 | Touch XL   (only used on TS modules)
  // #24/D15/A1 |  PC1 | Touch XR   (only used on TS modules)
  // #25/D16/A2 |  PC2 | Touch YD   (only used on TS modules)
  // #26/D17/A3 |  PC3 | Touch YU   (only used on TS modules)
  //==========================================================================
  //Set up port B
  pinMode(RS_PIN, OUTPUT);
  pinMode(CS_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  pinMode(5, OUTPUT);
  //pinMode(SCK_PIN, OUTPUT);
  //pinMode(MOSI_PIN, OUTPUT);
  
  //Drive the ports to a reasonable starting state.
  CLR_RESET;
  CLR_RS;
  SET_CS;
  //CLR_MOSI;
  //CLR_SCK;

#if(SYNC == SYNC_MASTER)
    pinMode(SYNC_PIN_MASTER,OUTPUT);
    digitalWrite(SYNC_PIN_MASTER,LOW);
    pinMode(SYNC_PIN_SLAVE,INPUT);
    digitalWrite(SYNC_PIN_SLAVE,LOW);
#endif
#if(SYNC == SYNC_SLAVE)
    pinMode(SYNC_PIN_MASTER,INPUT);
    digitalWrite(SYNC_PIN_MASTER,LOW);
    pinMode(SYNC_PIN_SLAVE,OUTPUT);
    digitalWrite(SYNC_PIN_SLAVE,LOW);
#endif

  //Pin 7 is used for the SD card CS.
  pinMode(7, OUTPUT);

  //debug console
  Serial.begin(9600);
  Serial.print("setup()");

  // Initialize SPI. By default the clock is 4MHz.
  SPI.begin();
  //Bump the clock to 8MHz. Appears to be the maximum.
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));

  //Initialize the LCD controller
  Initialize_LCD();

  // For the Seeduino I am using, the default speed of SPI_HALF_SPEED
  // set in C:\Program Files (x86)\Arduino\libraries\SD\src\SD.cpp
  // results in a 4MHz clock.
  //
  // If you change this function call in SDClass::begin() of SD.cpp
  // from:
  //
  //  return card.init(SPI_HALF_SPEED, csPin) &&
  //         volume.init(card) &&
  //         root.openRoot(volume);
  //
  // to:
  //
  //  return card.init(SPI_FULL_SPEED, csPin) &&
  //         volume.init(card) &&
  //         root.openRoot(volume);
  //
  // That appears to make the SD library talk at 8MHz.
  //
  // Alas, it only improves a full-screen update from a dismally slow
  // time of 1.81 seconds to the nearly hopeless time of 1.35 seconds.
  //
 /* if (!SD.begin(7)) 
    {
    Serial.println("Card failed to initialize, or not present");
    }
  else
    {
    Serial.println("Card initialized.");
    }*/
    Serial.println("Setup complete");
  }
//============================================================================
void loop(){
  digitalWrite(5, LOW);
  delay(4000);
  digitalWrite(5, HIGH);
  delay(4000);
}
//============================================================================

