/*
 * @file	hx8357.c
 *
 *	Jimmy Sung
 */

#include <stdlib.h>
#include <string.h>
#include "hx8357.h"
#include "spi.h"

#define DISP_SPI	hspi1
#define RST_GPIO 	GPIOA
#define RST_PIN		GPIO_PIN_2
#define DC_GPIO		GPIOA
#define DC_PIN		GPIO_PIN_3

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))

// #define byte_flip(v)

static uint8_t initd[] = {
	HX8357_SWRESET,
	0x80 + 100 / 5, // Soft reset, then delay 10 ms
	HX8357D_SETC,
	3,
	0xFF,
	0x83,
	0x57,
	0xFF,
	0x80 + 500 / 5, // No command, just delay 300 ms
	HX8357_SETRGB,
	4,
	0x80,
	0x00,
	0x06,
	0x06, // 0x80 enables SDO pin (0x00 disables)
	HX8357D_SETCOM,
	1,
	0x25, // -1.52V
	HX8357_SETOSC,
	1,
	0x68, // Normal mode 70Hz, Idle mode 55 Hz
	HX8357_SETPANEL,
	1,
	0x05, // BGR, Gate direction swapped
	HX8357_SETPWR1,
	6,
	0x00, // Not deep standby
	0x15, // BT
	0x1C, // VSPR
	0x1C, // VSNR
	0x83, // AP
	0xAA, // FS
	HX8357D_SETSTBA,
	6,
	0x50, // OPON normal
	0x50, // OPON idle
	0x01, // STBA
	0x3C, // STBA
	0x1E, // STBA
	0x08, // GEN
	HX8357D_SETCYC,
	7,
	0x02, // NW 0x02
	0x40, // RTN
	0x00, // DIV
	0x2A, // DUM
	0x2A, // DUM
	0x0D, // GDON
	0x78, // GDOFF
	HX8357D_SETGAMMA,
	34,
	0x02,
	0x0A,
	0x11,
	0x1d,
	0x23,
	0x35,
	0x41,
	0x4b,
	0x4b,
	0x42,
	0x3A,
	0x27,
	0x1B,
	0x08,
	0x09,
	0x03,
	0x02,
	0x0A,
	0x11,
	0x1d,
	0x23,
	0x35,
	0x41,
	0x4b,
	0x4b,
	0x42,
	0x3A,
	0x27,
	0x1B,
	0x08,
	0x09,
	0x03,
	0x00,
	0x01,
	HX8357_COLMOD,
	1,
	0x55, // use 0x66 for 6-6-6 mode, use 0x55 for 5-6-5 mode
	HX8357_MADCTL,
	1,
	0xC0,
	HX8357_TEON,
	1,
	0x00, // TW off
	HX8357_TEARLINE,
	2,
	0x00,
	0x02,
	HX8357_SLPOUT,
	0x80 + 150 / 5, // Exit Sleep, then delay 150 ms
	HX8357_DISPON,
	0x80 + 50 / 5, // Main screen turn on, delay 50 ms
	0,             // END OF COMMAND LIST
};

void hx8357_disp_init() {
	// Hard reset the display first
	hx8357_disp_reset();

	uint8_t *init_prog = initd;
	uint8_t x, cmd, numArgs;

	while ((cmd = pgm_read_byte(init_prog++)) > 0) {
		if (cmd != 0xFF) {
			// writeCommand(cmd);
			hx8357_writeCommand(cmd);
		}

		x = pgm_read_byte(init_prog++);
		numArgs	= x & 0x7F;

		if (x & 0x80) {				// If the highest bit is set...
			HAL_Delay(numArgs * 5);	// numArgs is actually a delay time (5ms units)
		} else {					// Otherwise, issue args to command...
			while (numArgs--) {
				// spiWrite(pgm_read_bytes(init_prog++));
				HAL_SPI_Transmit(&DISP_SPI, init_prog++, 1, HAL_MAX_DELAY);
			}
		}
	}
}

void hx8357_disp_reset() {
	HAL_GPIO_WritePin(RST_GPIO, RST_PIN, GPIO_PIN_SET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(RST_GPIO, RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(RST_GPIO, RST_PIN, GPIO_PIN_SET);
}

void hx8357_writeCommand(uint8_t cmd) {
	HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&DISP_SPI, &cmd, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_SET);
}

void hx8357_disp_invert(unsigned char invert) {
	uint8_t inv_cmd = (invert == 0x00) ? HX8357_INVOFF : HX8357_INVON;
	hx8357_writeCommand(inv_cmd);
}

void hx8357_disp_setAddrWindow(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) {
	uint16_t x2 = (x1 + w - 1);
	uint16_t y2 = (y1 + h - 1);
	uint8_t xStream[] = {
		(uint8_t) ((x1 & ((uint16_t) 0xFF00)) >> 8), 
		(uint8_t) (x1 & 0xFF),
		(uint8_t) ((x2 & ((uint16_t) 0xFF00)) >> 8),
		(uint8_t) (x2 & 0xFF)
	};

	uint8_t yStream[] = {
		(uint8_t) ((y1 & ((uint16_t) 0xFF00)) >> 8), 
		(uint8_t) (y1 & 0xFF),
		(uint8_t) ((y2 & ((uint16_t) 0xFF00)) >> 8),
		(uint8_t) (y2 & 0xFF)
	};

	hx8357_writeCommand(HX8357_CASET);	// Column address set
	HAL_SPI_Transmit(&DISP_SPI, xStream, 4, HAL_MAX_DELAY);
	hx8357_writeCommand(HX8357_PASET);	// Row address set
	HAL_SPI_Transmit(&DISP_SPI, yStream, 4, HAL_MAX_DELAY);
	hx8357_writeCommand(HX8357_RAMWR);	// Write to RAM
}

void hx8357_writePixels(uint16_t *colors, uint32_t len) {
	 uint8_t *colorStream = (uint8_t*)(colors);
	 HAL_SPI_Transmit(&DISP_SPI, colorStream, (2 * len), HAL_MAX_DELAY);
}

/* 
	@brief	Writes a 2-byte color many times
	@param	color	The 16-bit 5-6-5 color to draw
	@param	len		The number of pixels to draw
*/
void hx8357_writeColor(uint16_t color, uint32_t len) {
	if (!len) return;	// Avoid 0-byte transfers

	DISP_SPI.Init.DataSize = SPI_DATASIZE_16BIT;

	static uint32_t temp[32];
	color = ((color & 0xFF) << 8) | ((color & 0xFF00) >> 8);
	uint32_t 		c32 = color * 0x00010001;
	uint16_t		bufLen = (len < 32) ? len : 32;
	uint16_t		xferLen, fillLen;

	fillLen = (bufLen + 1) / 2;		// Round up to nearest 32-bit boundry
	for (int i = 0; i < fillLen; i++) {	// Fill temp[] with c32
		temp[i] = c32;
	}

	while (len) {
		xferLen = (bufLen < len) ? bufLen : len;	// How many pixels to pass
		HAL_SPI_Transmit(&DISP_SPI, (void*) temp, xferLen, HAL_MAX_DELAY);
		len -= xferLen;
	}

	DISP_SPI.Init.DataSize = SPI_DATASIZE_8BIT;
}

void hx8357_writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	if ((x >= HX8357_TFTWIDTH) || (y >= HX8357_TFTHEIGHT)) return;

	int16_t x2 = x + w - 1;
	int16_t y2 = y + h - 1;

	if ((x2 < 0) || (y2 < 0)) return;

	// Clip left/top
	if(x < 0) {
		x = 0;
		w = x2 + 1;
	}
	if(y < 0) {
		y = 0;
		h = y2 + 1;
	}

	// Clip right/bottom
    if(x2 >= HX8357_TFTWIDTH)  w = HX8357_TFTWIDTH  - x;
    if(y2 >= HX8357_TFTHEIGHT) h = HX8357_TFTHEIGHT - y;
    hx8357_disp_setAddrWindow(x, y, w, h);
    hx8357_writeColor(color, (int32_t)w * h);
}

void hx8357_writeHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
	hx8357_writeFillRect(x, y, w, 1, color);
}

void hx8357_writeVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
	hx8357_writeFillRect(x, y, 1, h, color);
}

void hx8357_writePixle(int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= HX8357_TFTWIDTH)) return;
	if ((y < 0) || (y >= HX8357_TFTHEIGHT)) return;

	uint8_t colorStream[] = {
		(uint8_t) ((color >> 8) & 0xFF),
		(uint8_t) (color & 0xFF)
	};

	hx8357_disp_setAddrWindow(x, y, 1, 1);
	HAL_SPI_Transmit(&DISP_SPI, colorStream, 2, HAL_MAX_DELAY);
}

uint16_t hx8357_color565(uint8_t red, uint8_t green, uint8_t blue) {
	return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
}

void hx8357_drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors, int16_t w, int16_t h) {
	int16_t x2, y2; // Lower-right coord
    if(( x             >= HX8357_TFTWIDTH ) ||      // Off-edge right
       ( y             >= HX8357_TFTHEIGHT) ||      // " top
       ((x2 = (x+w-1)) <  0      ) ||      // " left
       ((y2 = (y+h-1)) <  0)     ) return; // " bottom

	int16_t bx1=0, by1=0, // Clipped top-left within bitmap
            saveW=w;      // Save original bitmap width value
    if(x < 0) { // Clip left
        w  +=  x;
        bx1 = -x;
        x   =  0;
    }
    if(y < 0) { // Clip top
        h  +=  y;
        by1 = -y;
        y   =  0;
    }
    if(x2 >= HX8357_TFTWIDTH ) w = HX8357_TFTWIDTH  - x; // Clip right
    if(y2 >= HX8357_TFTHEIGHT) h = HX8357_TFTHEIGHT - y; // Clip bottom

    pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left

	setAddrWindow(x, y, w, h); // Clipped area

    while(h--) { // For each (clipped) scanline...
        //   writePixels(pcolors, w); // Push one (clipped) row
        HAL_SPI_Transmit(&DISP_SPI, (void*) pcolors, w, HAL_MAX_DELAY);
        pcolors += saveW; // Advance pointer by one full (unclipped) line
    }
}

/* ADAFRUIT GFX FUNCTIONS */

