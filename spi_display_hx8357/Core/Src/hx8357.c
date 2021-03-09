/*
 * @file	hx8357.c
 *
 *	Jimmy Sung
 */

#include <stdlib.h>
#include <string.h>
#include "hx8357.h"
#include "spi.h"
#include "glcdfont.c"
#include <math.h>

#define DISP_SPI	hspi1
#define RST_GPIO 	GPIOA
#define RST_PIN		GPIO_PIN_2
#define DC_GPIO		GPIOA
#define DC_PIN		GPIO_PIN_3

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))

uint16_t disp_width = HX8357_TFTWIDTH;
uint16_t disp_height = HX8357_TFTHEIGHT;
uint8_t rotation = 0;

/*! Text parameters
	@brief The following global variables are used to for drawing text on the display and should not be modified directly
*/
uint16_t cursor_x = 0;
uint16_t cursor_y = 0;
uint8_t textsize = 1;	// used for print() and its supporting functions
bool wrap = 0;
uint16_t textcolor = HX8357_BLACK;
uint16_t textbgcolor = HX8357_WHITE;
uint8_t margin_x = 0;
uint8_t margin_y = 0;
// #define byte_flip(v)

/*!	initd[]
	@brief	This array contains the necessary stream of commands and parameters to initialize the HX8357D LCD display driver.
*/
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

/*!	hx8357_disp_init
	@brief	This function initializes the HX8357D LCD display driver by sending over initd[].
*/
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


/*!	hz8357_disp_reset
	@brief	This function pulls the NRST pin on the HX8357D LCD display driver low,
			forcing the display to reset.
*/
void hx8357_disp_reset() {
	HAL_GPIO_WritePin(RST_GPIO, RST_PIN, GPIO_PIN_SET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(RST_GPIO, RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(RST_GPIO, RST_PIN, GPIO_PIN_SET);
}

/*!	hx8357_writeCommand
	@brief	This function sends a command to the HX8357D over SPI.
	TODO:	Implement DMA version. Current version is blocking.
	@param	cmd	A 1 byte command to send to the display.
*/
void hx8357_writeCommand(uint8_t cmd) {
	HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&DISP_SPI, &cmd, 1, HAL_MAX_DELAY);
	HAL_GPIO_WritePin(DC_GPIO, DC_PIN, GPIO_PIN_SET);
}

/*!	hx8357_disp_invert
	@brief	This function inverts the display colors if passed in true.
	@param  invert	True for invert, false for standard colors
*/
void hx8357_disp_invert(bool invert) {
	uint8_t inv_cmd = (!invert) ? HX8357_INVOFF : HX8357_INVON;
	hx8357_writeCommand(inv_cmd);
}

/*!	hx8357_disp_setAddrWindow
	@brief	This function sets the starting and ending location for the next stream of pixels.
		This function also sets up the driver to receive a pixel stream into its buffer immediately after
		this function call.
		Must be a rectangle.
	@param	x1  The starting x position
	@param 	y1  The starting y position
	@param 	w  	The width of the rectangle to draw
	@param 	h  	The height of the rectangle to draw
*/
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

/*!	hx8357_writePixels
	@brief	This function takes a packed array of pixel color information and sends it to the display driver.
		This function does multiple pixels at a time.
	@param colors  The packed array of pixel color values
	@param len  The number of pixels to deal with
*/
void hx8357_writePixels(uint16_t *colors, uint32_t len) {
	uint8_t *colorStream = (uint8_t*)(colors);
	HAL_SPI_Transmit(&DISP_SPI, colorStream, (2 * len), HAL_MAX_DELAY);
}

/*!
	@brief	Writes a 2-byte color many times
	@param	color  The 16-bit 5-6-5 color to draw
	@param	len  The number of pixels to draw
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

/*! hx8357_writeFillRect
	@brief This function draws a colored rectangle of size w*h at position (x,y).
	@param x  The starting x position of the rectangle
	@param y  The starting y position of the rectangle
	@param w  The width of the rectangle
	@param h  The height of the rectangle
	@param color  The color of the rectangle, must use packed 565 color format.
*/
void hx8357_writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
	if ((x >= disp_width) || (y >= disp_height)) return;

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
    if(x2 >= disp_width)  w = disp_width  - x;
    if(y2 >= disp_height) h = disp_height - y;
    hx8357_disp_setAddrWindow(x, y, w, h);
    hx8357_writeColor(color, (int32_t)w * h);
}

/*! hx8357_writeHLine 
	@brief	This function draws a colored horizontal line at position (x,y)
	@param x  The starting x position of the line
	@param y  The starting y position of the line
	@param len  The length of the line
	@param color  The color of the line, must use packed 565 color format.
*/
void hx8357_writeHLine(int16_t x, int16_t y, int16_t len, uint16_t color) {
	hx8357_writeFillRect(x, y, len, 1, color);
}

/*! hx8357_writeVLine
	@brief	This function draws a colored vertical line at position (x,y)
	@param x  The starting x position of the line
	@param y  The starting y position of the line
	@param len  The length of the line
	@param color  The color of the line, must use packed 565 color format.
*/
void hx8357_writeVLine(int16_t x, int16_t y, int16_t len, uint16_t color) {
	hx8357_writeFillRect(x, y, 1, len, color);
}

/*! hx8357_writePixel
	@brief	This function writes a single colored pixel
	@param x  The x position of the pixel
	@param y  The y position of the pixel 
*/
void hx8357_writePixel(int16_t x, int16_t y, uint16_t color) {
	if ((x < 0) || (x >= disp_width)) return;
	if ((y < 0) || (y >= disp_height)) return;

	uint8_t colorStream[] = {
		(uint8_t) ((color >> 8) & 0xFF),
		(uint8_t) (color & 0xFF)
	};

	hx8357_disp_setAddrWindow(x, y, 1, 1);
	HAL_SPI_Transmit(&DISP_SPI, colorStream, 2, HAL_MAX_DELAY);
}

/*! hx8357_color565
	@brief  This function converts 8-bit RGB values (separated) into a packed 16-bit color value
	@param red  An 8-bit red value
	@param green  An 8-bit green value
	@param blue  An 8-bit blue value
*/
uint16_t hx8357_color565(uint8_t red, uint8_t green, uint8_t blue) {
	return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | ((blue & 0xF8) >> 3);
}

/*! hx8357_disp_setRotation
	@brief  This function sets the displays rotation
	@param m  Rotation mode.
		0 - Portrait, SPI conn at bottom.
		1 - Landscape, SPI conn at right.
		2 - Portrait, SPI conn at top.
		3 - Landscape, SPI conn at left.
*/
void hx8357_disp_setRotation(uint8_t m) {
	rotation = m & 3; // can't be higher than 3

	switch (rotation) {
		case 0:
			m       	= MADCTL_MX | MADCTL_MY | MADCTL_RGB;
			disp_width  = HX8357_TFTWIDTH;
			disp_height = HX8357_TFTHEIGHT;
			break;
		case 1:
			m       	= MADCTL_MV | MADCTL_MY | MADCTL_RGB;
			disp_width  = HX8357_TFTHEIGHT;
			disp_height = HX8357_TFTWIDTH;
			break;
		case 2:
			m       	= MADCTL_RGB;
			disp_width  = HX8357_TFTWIDTH;
			disp_height = HX8357_TFTHEIGHT;
			break;
		case 3:
			m       	= MADCTL_MX | MADCTL_MV | MADCTL_RGB;
			disp_width  = HX8357_TFTHEIGHT;
			disp_height = HX8357_TFTWIDTH;
			break;
	}

	hx8357_writeCommand(HX8357_MADCTL);
	HAL_SPI_Transmit(&DISP_SPI, &m, 1, HAL_MAX_DELAY);
}

/***
 *  (ADAFRUIT) GFX FUNCTIONS 
 ***/

/*! hx8357_drawRGBBitmap
	@brief  This function draws an RGB bitmap
	TODO: Needs to be tested
	@param x  The starting x position of the image
	@param y  The starting y position of the image
	@param pcolors  The bitmap
	@param w  The width of the image
	@param h  The height of the image
*/
void hx8357_drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors, int16_t w, int16_t h) {
	int16_t x2, y2; // Lower-right coord
    if(( x             >= disp_width ) ||      // Off-edge right
       ( y             >= disp_height) ||      // " top
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
    if(x2 >= disp_width ) w = disp_width  - x; // Clip right
    if(y2 >= disp_height) h = disp_height - y; // Clip bottom

    pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left

	hx8357_disp_setAddrWindow(x, y, w, h); // Clipped area

    while(h--) { // For each (clipped) scanline...
        //   writePixels(pcolors, w); // Push one (clipped) row
        HAL_SPI_Transmit(&DISP_SPI, (void*) pcolors, w, HAL_MAX_DELAY);
        pcolors += saveW; // Advance pointer by one full (unclipped) line
    }
}

/*! hx8357_fillDisplay
	@brief  This function fills the display with one color
	@param color  The color to use
*/
void hx8357_fillDisplay(uint16_t color) {
	hx8357_writeFillRect(0, 0, disp_width, disp_height, color);
}

/*!	hx8357_drawChar
	@brief  This function draws a single character
	@param x  The starting x position of the char
	@param y  The starting y position of the char
	@param color  The color of the char
	@param bg  The background color of the char
	@param size  The size of the char
*/
void hx8357_drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
	// TODO: Add custom font handling
	if	((x >= disp_width)   || // Clip right
		 (y >= disp_height)  || // Clip bottom
		 ((x + 6 * size - 1) < 0) || // Clip left
		 ((y + 8 * size - 1) < 0)) { // Clip top
			return; // The text we want to display doesn't actually fit 
	}

	// There was an inline patch from Adafruit for an older version of glcdfont.c that was missing a character
	// This version of drawChar relies on the newer version of glcdfont.c, so the fix isn't implemented here
	// The fix in question goes something like this:
	// if (old_glcdfont && (c >= 176)) c++;
    
	for (int8_t i = 0; i < 5; i++) {
		uint8_t line = pgm_read_byte(&font[c * 5 + i]);

		for (int8_t j = 0; j < 8; j++, line >>= 1) {
			if (line & 1) {
				if (size == 1) {
					hx8357_writePixel(x+i, y+j, color);
				} else {
					hx8357_writeFillRect(x + i * size, y + j * size, size, size, color);
				}
			}
			else if (bg != color) {
				if (size == 1) {
					hx8357_writePixel(x+i, y+j, bg);
				} else {
					hx8357_writeFillRect(x + i * size, y + j * size, size, size, bg);
				}
			}
		}

		if (bg != color) {
			if (size == 1) {
				hx8357_writeVLine(x + 5, y, 8, bg);
			} else {
				hx8357_writeFillRect(x + 5 * size, y, size, 8 * size, bg);
			}
		}
	}
}

/*! hx8357_write
	@brief  This funcntion  
*/
void hx8357_write(uint8_t c) {
	// TODO: add handling of custom fonts

	if (c == '\n') {				// newline?	
		cursor_x = margin_x;		// reset x to zero,
		cursor_y += textsize * 8;	// advance y by one line
	}
	else if (c != '\r') {
		if (wrap && ((cursor_x + textsize * 6) > disp_width)) {	// Do we need to wrap lines?
			cursor_x = margin_x;
			cursor_y += textsize * 8;
		}
		hx8357_drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);	// draw the char
		cursor_x += textsize * 6;	// advance x by one char
	}
}

void hx8357_setCursor(uint16_t x, uint16_t y) {
	cursor_x = x;
	cursor_y = y;
}

uint16_t hx8357_getCursorX() {
	return cursor_x;
}

uint16_t hx8357_getCursorY() {
	return cursor_y;
}

void hx8357_setTextSize(uint8_t s) {
	textsize = (s > 0) ? s : 1;
}

void hx8357_setTextColor(uint16_t color) {
	textcolor = textbgcolor = color;
}

void hx8357_setTextBGColor(uint16_t color) {
	textbgcolor = color;
}

void hx8357_setTextWrap(bool w) {
	wrap = w;
}

void hx8357_setMarginX(uint8_t m) {
	margin_x = m;
}

/***
 * Print.cpp functions
 * License:
	Print.cpp - Base class that provides print() and println()
	Copyright (c) 2008 David A. Mellis.  All right reserved.
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
	
	Modified 23 November 2006 by David A. Mellis
 */
void printWrite (const char *str) {
	while (*str) {
		hx8357_write(*str++);
	}
}

void printChar(char c) {
	hx8357_write(c);
}

void printString(const char str[]) {
	printWrite(str);
}

void printUnsigned(unsigned long n, uint8_t base) {
	unsigned char buf[8 * sizeof(long)]; // Assumes 8-bit chars. 
	unsigned long i = 0;
	if (n == 0) {
		printChar('0');
		return;
	} 
	while (n > 0) {
		buf[i++] = n % base;
		n /= base;
	}
	for (; i > 0; i--) {
		printChar((char) (buf[i - 1] < 10 ? '0' + buf[i - 1] : 'A' + buf[i - 1] - 10));
	}
}

void printFloat(double number, uint8_t digits) {
	// Handle negative numbers
	if (number < 0.0) {
		printChar('-');
		number = -number;
	}

	// Round correctly so that print(1.999, 2) prints as "2.00"
	double rounding = 0.5;
	for (uint8_t i = 0; i < digits; ++i) {
		rounding /= 10.0;
	}
	number += rounding;

	// Extract the integer part of the number and print it
	unsigned long int_part = (unsigned long)number;
	double remainder = number - (double)int_part;
	printUnsigned(int_part, 10);

	// Print the decimal point, but only if there are digits beyond
	if (digits > 0) {
		printChar('.'); 
	}

	// Extract digits from the remainder one at a time
	while (digits-- > 0) {
		remainder *= 10.0;
		int toPrint = (int) remainder;
		printUnsigned(toPrint, 10);
		remainder -= toPrint; 
	} 
}