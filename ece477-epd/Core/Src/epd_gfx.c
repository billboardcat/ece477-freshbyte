//
// Created by Jimmy Sung on 3/14/21.
//
#include "epd_gfx.h"
#include "glcdfont.c"
#include "serial_print.h"

// Many (but maybe not all) non-AVR board installs define macros
// for compatibility with existing PROGMEM-reading AVR code.
// Do our own checks and defines here for good measure...

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif
#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const unsigned short *)(addr))
#endif
#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const unsigned long *)(addr))
#endif

// Pointers are a peculiar case...typically 16-bit on AVR boards,
// 32 bits elsewhere.  Try to accommodate both...

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
#define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
#define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                    \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  }
#endif

#define CHAR_BIT 8
//#define _abs(a) ((unsigned int) (((a) + ((a) >> (sizeof(int) * CHAR_BIT - 1))) ^ ((a) >> (sizeof(int) * CHAR_BIT - 1))))
#define _abs(a) (((a) < 0) ? (-(a)) : (a))

// These extern global variables are parameters of the display itself
extern uint8_t rotation;
extern uint16_t width, height;

// These global variables are only used by the "gfx library" itself
uint8_t textsize_x = 1;
uint8_t textsize_y = 1;
int16_t cursor_x = 0;
int16_t cursor_y = 0;
uint8_t margin_x = 0;
uint8_t margin_y = 0;
uint16_t text_color = EPD_BLACK;
uint16_t text_bg_color = EPD_WHITE;
bool wrap = false;

/*!
 * @brief Sets the rotation for the display.
 * @param x   0 thru 3 corresponding to 4 cardinal rotations.
 */
void set_rotation(uint8_t x) {
    rotation = (x & 3);
    switch (rotation) {
        case 0:
        case 2:
            width = EPD_WIDTH;
            height = EPD_HEIGHT;
            break;
        case 1:
        case 3:
            width = EPD_HEIGHT;
            height = EPD_WIDTH;
            break;
    }
}

/*!
 * @brief       Sets the scaling factors for the x and y axis of text.
 * @param   x   The desired scaling factor for the x-axis of a character.
 * @param   y   The desired scaling factor for the y-axis of a character.
 */
void set_text_size(uint8_t x, uint8_t y) {
    textsize_x = (x > 0) ? x : 1;
    textsize_y = (y > 0) ? y : 1;
}

/*!
 * @brief      Sets the scaling factors for the x and y axis to be the same value.
 * @param   s  The desired scaling factor for the x and y axis of a character.
 */
void set_text_scale(uint8_t s) { set_text_size(s, s); }

/*!
 * @brief       Sets the flag for text wrapping on the display.
 * @param   w   True for text wrapping. False for cutting-off text
 */
void set_text_wrap(bool w) { wrap = w; }

/*!
 * @brief       Sets the color for text to be printed in.
 * @param color The desired color
 */
void set_text_color(uint16_t color) { text_color = color; }

/*!
 * @brief       Sets the background color for text to be printed in.
 * @param color The desired color
 */
void set_text_bg_color(uint16_t color) { text_bg_color = color; }

/*!
 * @brief       Sets the cursor to a new location on the display.
 * @param   x   The x coordinate of the new location
 * @param   y   The y coordinate of the new location
 */
void set_cursor(uint16_t x, uint16_t y) {
    cursor_x = x;
    cursor_y = y;
}

/*!
 * @return Returns the current x coordinate of the cursor
 */
int16_t get_x_cursor() { return cursor_x; }

/*!
 * @return Returns the current Y coordinate of the cursor
 */
int16_t get_y_cursor() { return cursor_y; }

/*!
 * @brief       Sets a horizontal offset from the left of the display for all lines of text.
 * @param   m   The number of pixels to offset by
 */
void set_x_margin(uint8_t m) { margin_x = m; }

/*!
 * @brief       Sets a vertical offset from the top of the display for all text.
 * @param   m   The number of pixels to offset by
 */
void set_y_margin(uint8_t m) { margin_y = m; }

uint8_t get_x_margin() { return margin_x; }

uint8_t get_y_margin() { return margin_y; }

/*!
    @brief    Write a line.  Bresenham's algorithm - thx wikpedia
    @param    x0  Start point x coordinate
    @param    y0  Start point y coordinate
    @param    x1  End point x coordinate
    @param    y1  End point y coordinate
    @param    color 16-bit 5-6-5 Color to draw with
*/
void write_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = _abs(y1 - y0) > _abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = _abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0 <= x1; x0++) {
        if (steep) {
            draw_pixel(y0, x0, color);
        } else {
            draw_pixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

/*!
 * @brief       Writes a perfectly vertical line.
 * @param x     The top-most x coordinate
 * @param y     The top-most y coordinate
 * @param h     The height in pixels
 * @param color The 16-bit 5-6-5 color to use
 */
void write_fast_vLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    write_line(x, y, x, y + h - 1, color);
//    fill_rect(x, y, 1, h, color);
}

/*!
 * @brief       Writes a perfectly horizontal line
 * @param x     The left-most x coordinate
 * @param y     The left-most y coordinate
 * @param w     The width in pixels
 * @param color The 16-bit 5-6-5 color to use
 */
void write_fast_hLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    write_line(x, y, x + w - 1, y, color);
//    fill_rect(x, y, w, 1, color);
}

/*!
 * @brief       Writes a rectangle completely filled with one color
 * @param x     The top-left corner x coordinate
 * @param y     The top-left corner y coordinate
 * @param w     The width in pixels
 * @param h     The height in pixels
 * @param color The 16-bit 5-6-5 color to use
 */
void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    for (int16_t i = x; i < (x + w); i++) {
        write_fast_vLine(i, y, h, color);
    }
}

/*!
 * @brief       Fills the display with a single color
 * @param color The 16-bit 5-6-5 color to fill the display with
 */
void fill_screen(uint16_t color) {
    fill_rect(0, 0, EPD_WIDTH, EPD_HEIGHT, color);
}

/*!
 * @brief       This function draws a line
 * @param x0    The starting x coordinate
 * @param y0    The starting y coordinate
 * @param x1    The ending x coordinate
 * @param y1    The ending y coordinate
 * @param color The 16-bit 5-6-5 color to draw with
 */
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    if (x0 == x1) {
        if (y0 > y1) {
            _swap_int16_t(y0, y1);
        }
        write_fast_vLine(x0, y0, y1 - y0 + 1, color);
    } else if (y0 == y1) {
        if (x0 > x1) {
            _swap_int16_t(x0, x1);
        }
        write_fast_hLine(x0, y0, x1 - x0 + 1, color);
    } else {
        write_line(x0, y0, x1, y1, color);
    }
}

// TEXT- AND CHARACTER-HANDLING FUNCTIONS ----------------------------------
/*!
 * @brief This function serves as the "back-end" of write().
 *          It calculates and draws a single character to the display or display buffer.
 * @param x         The starting x position of the char
 * @param y         The starting y position of the char
 * @param c         The char
 * @param color     The color of the char
 * @param bg        The background color of the char
 * @param size_x    The width of the char
 * @param size_y    The height of the char
 */
void draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y) {
    // TODO: add handling for non-classic fonts (if possible)
    if ((x >= width) ||                 // Clip right
        (y >= height) ||                // Clip bottom
        ((x + 6 * size_x - 1) < 0) ||   // Clip left
        ((y + 8 * size_y - 1) < 0)) {   // Clip top
        serial_println("CLIPPING TEXT");
        return;
    }

    // There was an inline patch from Adafruit for an older version of glcdfont.c that was missing a character
    // This version of drawChar relies on the newer version of glcdfont.c, so the fix isn't implemented here
    // The fix in question goes something like this:
    // if (old_glcdfont && (c >= 176)) c++;

    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = pgm_read_byte(&font[c * 5 + i]);

        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1) {
                if (size_x == 1 && size_y == 1) {
                    draw_pixel(x + i, y + j, color);
                } else {
                    fill_rect(x + i * size_x, y + j * size_y, size_x, size_y, color);
                }
            } else if (bg != color) {
                if (size_x == 1 && size_y == 1) {
                    draw_pixel(x + i, y + j, bg);
                } else {
                    fill_rect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
                }
            }
        } // End inner for loop
    } // End outer for loop

    if (bg != color) {
        if (size_x == 1 && size_y == 1) {
            write_fast_vLine(x + 5, y, 8, bg);
        } else {
            fill_rect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
        }
    }
}

/*!
 * @brief This function draws a character to the display at the current location of the cursor.
 * @param c
 */
void write(uint8_t c) {
    // TODO: add handling for non-classic fonts (if possible)

    if (c == '\n') {                // Newline?
        cursor_x = margin_x;        // Reset x to the margin
        cursor_y += textsize_y * 8; // Advance y by one line
    } else if (c != '\r') {         // Ignore carriage returns
        if (wrap && ((cursor_x + textsize_x * 6) > width)) {    // Off right?
            cursor_x = margin_x;        // Reset x to the margin
            cursor_y += textsize_y * 8; // Advance y one line
        }

        if ((cursor_y + textsize_y * 8) > height) { // Off bottom?
            cursor_y = margin_y;                    // Reset y to the margin
        }

        draw_char(cursor_x, cursor_y, c, text_color, text_bg_color, textsize_x, textsize_y);
        cursor_x += textsize_x * 6; // Advance x by one character
    }
}

void draw_bitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color) {
    int16_t byteWidth = (w + 7) / 8; // Bitmap scanline pad = whole byte
    uint8_t byte = 0;

//    startWrite();
    for (int16_t j = 0; j < h; j++, y++) {
        for (int16_t i = 0; i < w; i++) {
            if (i & 7)
                byte <<= 1;
            else
                byte = pgm_read_byte(&bitmap[j * byteWidth + i / 8]);
            if (byte & 0x80)
                draw_pixel(x + i, y, color); // writePixel(x + i, y, color);
        }
    }
//    endWrite();
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

/*!
 * @brief Print a *char string to the display
 * @param str   The string to print
 */
void printWrite (const char *str) {
	while (*str) {
		write(*str++);
	}
}

/*!
 * @brief Print a character to the display
 * @param c     The character to print
 */
void printChar(char c) {
	write(c);
}

/*!
 * @brief Print a char[] string to the display
 * @param str   The string to primt
 */
void printString(const char str[]) {
	printWrite(str);
}

/*!
 * @brief Print an unsigned value to the display
 * @param n     The unsigned value
 * @param base  The base of the unsigned value
 */
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

/*!
 * @brief Print a floating point value to the display
 * @param number    The floating point value
 * @param digits    The number of digits to print after the decimal point
 */
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