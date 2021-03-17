//
// Created by Jimmy Sung on 3/14/21.
//

#ifndef ECE477_EPD_EPD_GFX_H
#define ECE477_EPD_EPD_GFX_H

#include "epd.h"

// Setters/Getters
void set_rotation(uint8_t);
void set_text_scale(uint8_t);
void set_text_size(uint8_t, uint8_t);
void set_text_wrap(bool);
void set_text_color(uint16_t);
void set_text_bg_color(uint16_t color);
void set_cursor(uint16_t x, uint16_t y);
int16_t get_x_cursor();
int16_t get_y_cursor();

void set_x_margin(uint8_t m);
void set_y_margin(uint8_t m);
uint8_t get_x_margin();
uint8_t get_y_margin();

// Basic drawing functions
void write_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void write_fast_vLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void write_fast_hLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void fill_screen(uint16_t color);
void draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void draw_bitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);

// Basic text functions
void draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
void write(uint8_t c);
void printWrite (const char *str);
void printChar (char c);
void printString(const char str[]);
void printUnsigned(unsigned long n, uint8_t base);
void printFloat(double number, uint8_t digits);

#endif //ECE477_EPD_EPD_GFX_H
