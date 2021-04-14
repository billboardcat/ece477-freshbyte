#ifndef __EPD_H__
#define __EPD_H__

#include <stdint.h>
#include <stdbool.h>

#define EPD_RAM_BW 0x10
#define EPD_RAM_RED 0x13

#define IL91874_PANEL_SETTING 0x00
#define IL91874_POWER_SETTING 0x01
#define IL91874_POWER_OFF 0x02
#define IL91874_POWER_OFF_SEQUENCE 0x03
#define IL91874_POWER_ON 0x04
#define IL91874_POWER_ON_MEASURE 0x05
#define IL91874_BOOSTER_SOFT_START 0x06
#define IL91874_DEEP_SLEEP 0x07
#define IL91874_DTM1 0x10
#define IL91874_DATA_STOP 0x11
#define IL91874_DISPLAY_REFRESH 0x12
#define IL91874_PDTM1 0x14
#define IL91874_PDTM2 0x15
#define IL91874_PDRF 0x16
#define IL91874_LUT1 0x20
#define IL91874_LUTWW 0x21
#define IL91874_LUTBW 0x22
#define IL91874_LUTWB 0x23
#define IL91874_LUTBB 0x24
#define IL91874_PLL 0x30
#define IL91874_CDI 0x50
#define IL91874_RESOLUTION 0x61
#define IL91874_VCM_DC_SETTING 0x82

#define EPD_WIDTH 264
#define EPD_HEIGHT 176

#define EPD_CS_LOW  HAL_GPIO_WritePin(EPD_CS_GPIO_Port, EPD_CS_Pin, GPIO_PIN_RESET)
#define EPD_CS_HIGH HAL_GPIO_WritePin(EPD_CS_GPIO_Port, EPD_CS_Pin, GPIO_PIN_SET)
#define EPD_DC_LOW  HAL_GPIO_WritePin(EPD_DC_GPIO_Port, EPD_DC_Pin, GPIO_PIN_RESET)
#define EPD_DC_HIGH HAL_GPIO_WritePin(EPD_DC_GPIO_Port, EPD_DC_Pin, GPIO_PIN_SET)

#define EPD_swap(a, b)                                                         \
  {                                                                            \
    int16_t t = a;                                                             \
    a = b;                                                                     \
    b = t;                                                                     \
  } ///< simple swap function

void epd_reset(void);
uint8_t epd_command(uint8_t c, bool end);
void epd_pCommand(uint8_t c, const uint8_t *buf, uint16_t len);
void epd_commandList(const uint8_t *init_code);

// From Adafruit_IL91874.cpp/.h
void epd_data(const uint8_t *buf, uint16_t len);
void epd_busy(void);
void epd_update(void);
void epd_powerUp(void);
void epd_powerDown(void);
uint8_t epd_writeRAMcommand(uint8_t index);
void epd_init(bool sram_enabled);

// From Adafruit_EPD.cpp/.h
enum {
    EPD_WHITE, ///< white color
    EPD_BLACK, ///< black color
    EPD_RED,   ///< red color
    EPD_GRAY,  ///< gray color ('red' on grayscale)
    EPD_DARK,  ///< darker color
    EPD_LIGHT, ///< lighter color
    EPD_NUM_COLORS
};

void set_black_buffer(int8_t index, bool inverted);
void set_color_buffer(int8_t index, bool inverted);
void clear_buffer(void);
void clear_display(void);
void display(bool sleep);
void write_SRAM_to_epd(uint16_t sram_addr, uint32_t buffer_size, uint8_t epd_location, bool invert);
void write_RAM_to_epd(uint8_t *frame_buffer, uint32_t buffer_size, uint8_t epd_location, bool invert);
uint8_t spi_transfer(uint8_t d);
void draw_pixel(int16_t x, int16_t y, uint16_t color);

#endif