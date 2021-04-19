#include <stdlib.h>
#include <string.h>
#include "epd.h"
#include "main.h"
#include "spi.h"
#include "serial_print.h"
#include "stm32l0xx_hal.h"
#include "sram.h"

#define EPD_SPI     hspi1
#define BUSY_WAIT   500
#define REFRESH_WAIT 13000
//#define EPD_USE_COLOR
// Display parameters
bool use_nrst = false;
bool use_busy = false;
bool use_sram = true;
bool single_byte_tx = false;

bool black_buffer_inverted = false;
bool color_buffer_inverted = false;

//uint32_t buffer1_size;
//uint8_t *buffer1;
//uint8_t *black_buffer;  // On-chip ram pointers for buffers
//uint16_t buffer1_addr;
//uint16_t black_buffer_addr; // Ext. sram address offsets for the color
//
//#ifdef EPD_USE_COLORS
//uint32_t buffer2_size;
//uint8_t *buffer2;
//uint8_t *color_buffer; // On-chip ram pointers for buffers
//uint16_t buffer2_addr;
//uint16_t color_buffer_addr; // Ext. sram address offsets for the color
//#endif

uint8_t partials_since_last_full_update = 0;
uint8_t rotation = 2;
uint8_t layer_colors[EPD_NUM_COLORS];
const uint8_t *epd_init_code = NULL;
uint16_t width;
uint16_t height;

// define the colors of the display
// clang-format off
static const uint8_t ti_270c44_tri_init_code[] = {
        IL91874_BOOSTER_SOFT_START, 3, 0x07, 0x07, 0x17,
        IL91874_POWER_ON, 0,
        0xFF, 200,
        IL91874_PANEL_SETTING, 1, 0x0F, // OTP lut
        IL91874_PDRF, 1, 0x00,

        0xF8, 2, 0x60, 0xA5, // boost
        0xF8, 2, 0x73, 0x23, // boost
        0xF8, 2, 0x7C, 0x00, // boost

        0xFE // EOM
};

// clang-format on
const unsigned char lut_vcomDC[] = {
        0x00, 0x00, 0x00, 0x1A, 0x1A, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A,
        0x00, 0x00, 0x08, 0x00, 0x0E, 0x01, 0x0E, 0x01, 0x10, 0x00, 0x0A,
        0x0A, 0x00, 0x00, 0x08, 0x00, 0x04, 0x10, 0x00, 0x00, 0x05, 0x00,
        0x03, 0x0E, 0x00, 0x00, 0x0A, 0x00, 0x23, 0x00, 0x00, 0x00, 0x01};

// R21H
const unsigned char lut_ww[] = {
        0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01, 0x40, 0x0A, 0x0A, 0x00, 0x00,
        0x08, 0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10, 0x80, 0x0A, 0x0A, 0x00,
        0x00, 0x08, 0x00, 0x04, 0x10, 0x00, 0x00, 0x05, 0x00, 0x03, 0x0E,
        0x00, 0x00, 0x0A, 0x00, 0x23, 0x00, 0x00, 0x00, 0x01};

// R22H	r
const unsigned char lut_bw[] = {
        0xA0, 0x1A, 0x1A, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x0A, 0x00, 0x00,
        0x08, 0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10, 0x90, 0x0A, 0x0A, 0x00,
        0x00, 0x08, 0xB0, 0x04, 0x10, 0x00, 0x00, 0x05, 0xB0, 0x03, 0x0E,
        0x00, 0x00, 0x0A, 0xC0, 0x23, 0x00, 0x00, 0x00, 0x01};

// R23H	w
const unsigned char lut_bb[] = {
        0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01, 0x40, 0x0A, 0x0A, 0x00, 0x00,
        0x08, 0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10, 0x80, 0x0A, 0x0A, 0x00,
        0x00, 0x08, 0x00, 0x04, 0x10, 0x00, 0x00, 0x05, 0x00, 0x03, 0x0E,
        0x00, 0x00, 0x0A, 0x00, 0x23, 0x00, 0x00, 0x00, 0x01};

// R24H	b
const unsigned char lut_wb[] = {
        0x90, 0x1A, 0x1A, 0x00, 0x00, 0x01, 0x20, 0x0A, 0x0A, 0x00, 0x00,
        0x08, 0x84, 0x0E, 0x01, 0x0E, 0x01, 0x10, 0x10, 0x0A, 0x0A, 0x00,
        0x00, 0x08, 0x00, 0x04, 0x10, 0x00, 0x00, 0x05, 0x00, 0x03, 0x0E,
        0x00, 0x00, 0x0A, 0x00, 0x23, 0x00, 0x00, 0x00, 0x01};

/*! 
    @brief This function sends an EPD command with no data
    @param c    The command to send
    @param end  If true, the CS pin will be pulled high following the transaction. Pass true for Adafruit functions that don't define it during call!
    @return     A byte of data read back from the EPD.
*/
uint8_t epd_command(uint8_t c, bool end) {
    EPD_CS_HIGH;    // Ensure the CS pin is high before starting the transaction
    EPD_DC_LOW;     // Set DC low to indicate that a command is being sent
    EPD_CS_LOW;     // Set CS low to indicate the start of the transaction

    uint8_t rxData;
    HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&EPD_SPI, &c, &rxData, 1, HAL_MAX_DELAY);

    if (status != HAL_OK) {
        char *err;
        switch (status) {
            case HAL_BUSY:
                err = "EPD_COMMAND: FAILED TO TX/RX - HAL_BUSY";
                serial_println(err);
                break;
            case HAL_TIMEOUT:
                err = "EPD_COMMAND: FAILED TO TX/RX - HAL_TIMEOUT";
                serial_println(err);
                break;
            case HAL_ERROR:
                err = "EPD_COMMAND: FAILED TO TX/RX - HAL_ERROR";
                serial_println(err);
                break;
            default:
                err = "EPD_COMMAND: FAILED TO TX/RX - UNKNOWN ERROR";
                serial_println(err);
                break;
        }
    }

    if (end) {
        EPD_CS_HIGH;
    }

    return rxData;
}

/*!
    @brief This function sends data to the EPD for an ONGOING transaction
    @param buf the buffer of data to send
    @param len the length of the data buffer
*/
void epd_data(const uint8_t *buf, uint16_t len) {
    EPD_DC_HIGH;
    HAL_SPI_Transmit(&EPD_SPI, buf, len, HAL_MAX_DELAY);
    EPD_CS_HIGH;
}

/*!
    @brief This function sends an EPD command with data
    @param c the command to send
    @param buf the buffer of data to send
    @param len the length of the data buffer
*/
void epd_pCommand(uint8_t c, const uint8_t *buf, uint16_t len) {
    epd_command(c, false);
    epd_data(buf, len);
}

/*!
 * @brief This function is used by epd_powerUp() to send over the display initialization code
 * @param init_code
 */
void epd_commandList(const uint8_t *init_code) {
    uint8_t buf[64];

    while (*init_code != 0xFE) {
        uint8_t cmd = *init_code;
        init_code++;
        uint8_t num_args = *init_code;
        init_code++;
        if (cmd == 0xFF) {
            epd_busy();
            HAL_Delay(num_args);
            continue;
        }
//        if (num_args > sizeof(buf)) {
//            serial_println("ERROR - buf not large enough!");
//            while (1) {
//                HAL_Delay(HAL_MAX_DELAY);
//            }
//        }

        for (int i = 0; i < num_args; i++) {
            buf[i] = *init_code;
            init_code++;
        }

        epd_pCommand(cmd, buf, num_args);
    }
}

/*!
 * @brief This function performs a hardware reset on the display if EPD_NRST_PIN is defined
 */
void epd_reset() {
#ifdef EPD_NRST_Pin
    if (use_nrst) {
        HAL_GPIO_WritePin(EPD_NRST_GPIO_Port, EPD_NRST_Pin, GPIO_PIN_SET);
        HAL_Delay(10);
        HAL_GPIO_WritePin(EPD_NRST_GPIO_Port, EPD_NRST_Pin, GPIO_PIN_RESET);
        HAL_Delay(10);
        HAL_GPIO_WritePin(EPD_NRST_GPIO_Port, EPD_NRST_Pin, GPIO_PIN_SET);
        HAL_Delay(10);
    }
#endif
}

/*!
 * @brief If EPD_BUSY_Pin is defined, this function blocks execution while the EPD is busy. Else, it simply waits.
 */
void epd_busy() {
#ifdef EPD_BUSY_Pin
    if (use_busy) {
        while(HAL_GPIO_ReadPin(EPD_BUSY_GPIO_Port, EPD_BUSY_Pin)) {
            HAL_Delay(1);
        }
    } else {
        HAL_Delay(BUSY_WAIT);
    }
#else
    HAL_Delay(BUSY_WAIT);
#endif
}

/*!
 * @brief This function sends the display refresh command to the EPD
 */
void epd_update() {
    epd_command(IL91874_DISPLAY_REFRESH, true);
    HAL_Delay(100);
    epd_busy();
    if (!use_busy) {
        HAL_Delay(REFRESH_WAIT);
    }
}

/*!
 * @brief This function performs the EPD power on sequence.
 */
void epd_powerUp() {
    uint8_t buf[5];

    epd_reset();  // TODO: Implement this later for future use. Would be helpful to have.
    HAL_Delay(200);
    const uint8_t *init_code = epd_init_code;

    if (init_code != NULL) {
      epd_commandList(init_code);

      buf[0] = (EPD_HEIGHT >> 8) & 0xFF;
      buf[1] = EPD_HEIGHT & 0xFF;
      buf[2] = (EPD_WIDTH >> 8) & 0xFF;
      buf[3] = EPD_WIDTH & 0xFF;
      epd_pCommand(IL91874_RESOLUTION, buf, 4);

      buf[0] = 0x00;
      epd_pCommand(IL91874_PDRF, buf, 1);
    }
}

/*!
 * @brief This function performs the EPD power down sequence.
 *          If EPD_NRST_Pin is defined, the EPD will be put in a deep sleep mode to further conserve power.
 */
void epd_powerDown() {
    uint8_t buf[1];

    buf[0] = 0xF7;
    epd_pCommand(IL91874_CDI, buf, 1);

    // power off
    epd_command(IL91874_POWER_OFF, true);
    epd_busy();

    // Only deep sleep if we can get out of it
    if (use_nrst) {
        buf[0] = 0xA5;
        epd_pCommand(IL91874_DEEP_SLEEP, buf, 1);
    }
}

/*!
 * @brief This function sends the relevant display draw command to either draw in B/W or Color
 * @param index     Set to 0 for black and white drawing. Set to 1 for color drawing.
 * @return          A byte of data from the EPD.
 */
uint8_t epd_writeRAMcommand(uint8_t index) {
    if (index == 0) {
        return epd_command(EPD_RAM_BW, false);
    }
    if (index == 1) {
        return epd_command(EPD_RAM_RED, false);
    }

    return 0;
}

/* ===== ADAFRUIT_EPD.CPP FUNCTIONS ===== */
/*!
 * @brief This function sets the global variables for black buffer. Can be used to swap buffers b/t colors.
 *          NOTE: Ability to swap buffers requires that EPD_USE_COLOR be defined.
 * @param index     The buffer to select for black.
 * @param inverted  Whether or not to treat the colors in the buffer as inverted.
 */
void set_black_buffer(int8_t index, bool inverted) {
    if (index == 0) {
        if (use_sram) {
            black_buffer_addr = buffer1_addr;
        } else {
            black_buffer = buffer1;
        }
    }
#ifdef EPD_USE_COLOR
    if (index == 1) {
        if (use_sram) {
            black_buffer_addr = buffer2_addr;
        } else {
            black_buffer = buffer2;
        }
    }
#endif
    black_buffer_inverted = inverted;
}

/*!
 * @brief This function sets the global variable for the color buffer. Can be used to swap buffers b/t colors.
 *          NOTE: This function is only active when EPD_USE_COLOR is defined.
 * @param index     The buffer to select for color.
 * @param inverted  Whether or not to treat the colors in the buffer as inverted.
 */
void set_color_buffer(int8_t index, bool inverted) {
#ifdef EPD_USE_COLOR
    if (index == 0) {
        if (use_sram) {
            color_buffer_addr = buffer1_addr;
        } else {
            color_buffer = buffer1;
        }
    }
    if (index == 1) {
        if (use_sram) {
            color_buffer_addr = buffer2_addr;
        } else {
            color_buffer = buffer2;
        }
    }
    color_buffer_inverted = inverted;
#endif
}

/*!
 * @brief This function performs the initialization sequence for the EPD.
 * @param sram_enabled  A boolean that indicates whether or not the use the EPD's external SRAM module.
 */
void epd_init(bool sram_enabled) {
    buffer1_size = ((uint32_t) EPD_WIDTH * (uint32_t) EPD_HEIGHT) / 8;  // Calculate the (first) buffer's size
    use_sram = sram_enabled;                                            // Set the SRAM usage flag

#ifdef EPD_USE_COLOR
    buffer2_size = buffer1_size;

    if (use_sram) {                     // Set up buffers for SRAM usage
        buffer1_addr = 0;               // First buffer's address
        buffer2_addr = buffer1_size;    // Second buffer's address
        buffer1 = buffer2 = NULL;       // Set MCU RAM buffer pointers to NULL
    } else {                                        // Set up buffers for MCU RAM usage
        buffer1 = (uint8_t *) malloc(buffer1_size); // First buffer's address
        buffer2 = (uint8_t *) malloc(buffer2_size); // Second buffer's address
    }
#else
    if (use_sram) {         // Set up the buffer for SRAM usage
        buffer1_addr = 0;   // The buffer's address
        buffer1 = NULL;     // Set MCU RAM buffer pointer to NULL
    } else {                                        // Set up buffer for MCU RAM usage
        buffer1 = (uint8_t *) malloc(buffer1_size); // The buffer's address
    }
#endif

    single_byte_tx = true;  // Seems like this EPD requires CS to go high b/t each byte, ergo single byte transactions

    if (use_sram) { // If using SRAM, set the SRAM module to run in sequential mode
        sram_write8(0, K640_SEQUENTIAL_MODE, MCPSRAM_WRSR);
    }

    EPD_CS_HIGH;                                // Make sure CS starts high.
    epd_reset();                                // reset the display, if at all possible.
    epd_powerDown();                            // Turn off the display.
    epd_init_code = ti_270c44_tri_init_code;    // Set the pointer to the correct initialization code.

    // Setup the black and color buffers to buffer1 and buffer2 respectively
    set_black_buffer(0, false);
#ifdef EPD_USE_COLOR
    set_color_buffer(1, false);
#endif

    // Initialize layer_colors[] LUT
    layer_colors[EPD_WHITE] = 0b00;
    layer_colors[EPD_BLACK] = 0b01;
    layer_colors[EPD_RED] = 0b10;
    layer_colors[EPD_GRAY] = 0b10;
    layer_colors[EPD_LIGHT] = 0b00;
    layer_colors[EPD_DARK] = 0b01;

    // Set the display width, height, and rotation to their default values.
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
    rotation = 2;           // Set rotation to 2 since it matches the push-buttons on the shield
}

/*!
 * @brief This function clears the display buffer(s)
 */
void clear_buffer() {
    if (use_sram) {
        if (black_buffer_inverted) {
            sram_erase(black_buffer_addr, buffer1_size, 0xFF);
        } else {
            sram_erase(black_buffer_addr, buffer1_size, 0x00);
        }
#ifdef EPD_USE_COLOR
        if (color_buffer_inverted) {
            sram_erase(color_buffer_addr, buffer2_size, 0xFF);
        } else {
            sram_erase(color_buffer_addr, buffer2_size, 0x00);
        }
#endif
    } else {
        if (black_buffer) {
            if (black_buffer_inverted) {
                memset(black_buffer, 0xFF, buffer1_size);
            } else {
                memset(black_buffer, 0x00, buffer1_size);
            }
        }
#ifdef EPD_USE_COLOR
        if (color_buffer) { // The the color buffer is in place. EPD_USE_COLOR must be defined (i.e. MCU must have enough RAM.)
            if (color_buffer_inverted) {
                memset(color_buffer, 0xFF, buffer2_size);
            } else {
                memset(color_buffer, 0x00, buffer2_size);
            }
        }
#endif
    }
}

/*!
 * @brief This is a helper function for write_SRAM_to_epd().
 *          It transfers a byte to the EPD and reads a byte from the external SRAM
 * @param d     The byte to send to the EPD.
 * @return      The next byte to send to the EPD.
 */
uint8_t spi_transfer(uint8_t d) {
    uint8_t b[1] = {0x00};
    if (single_byte_tx) {
        EPD_CS_LOW;
        HAL_SPI_TransmitReceive(&EPD_SPI, &d, b, 1, HAL_MAX_DELAY);
        EPD_CS_HIGH;
        return b[0];
    } else {
        HAL_SPI_TransmitReceive(&EPD_SPI, &d, b, 1, HAL_MAX_DELAY);
        return b[0];
    }
}

/*!
 * @brief This function moves data from the SRAM display buffer to the EPD itself in order to draw and image.
 * @param sram_addr     The starting address of the buffer in SRAM.
 * @param buffer_size   The size of the buffer.
 * @param epd_location  The EPD location to write to. 0 writes to black, 1 writes to color.
 * @param invert        Whether or not to invert the image.
 */
void write_SRAM_to_epd(uint16_t sram_addr, uint32_t buffer_size, uint8_t epd_location, bool invert) {
    uint8_t c;
    sram_csLow();
    // send read command
    spi_transfer(MCPSRAM_READ);
    // send address
    spi_transfer(sram_addr >> 8);
    spi_transfer(sram_addr & 0xFF);

    // first data byte from SRAM will be transferred in at the same time as the EPD command is transferred out
    // note: calling epd_writeRAMcommand will start an SPI tx with the EPD
    c = epd_writeRAMcommand(epd_location);

    EPD_DC_HIGH;
    for (uint32_t i = 0; i < buffer_size; i++) {
        c = (invert) ? ~c : c;  // Invert the data if need be
        c = spi_transfer(c);    // Tx the current byte and Rx the next

        serial_printf("0x%x, ", c);
        if (i % 32 == 31) {
            serial_printf("\n$%x: ", i);
        }
    }
    EPD_CS_HIGH;
    sram_csHigh();
}

/*!
 * @brief This function moves data from the MCU RAM display buffer to the EPD itself in order to draw an image.
 * @param frame_buffer  The display buffer to transfer to the EPD.
 * @param buffer_size   The size of the display buffer.
 * @param epd_location  The EPD location to write to. 0 writes to black, 1 writes to color.
 * @param invert        Whether or not to invert the image.
 */
void write_RAM_to_epd(uint8_t *frame_buffer, uint32_t buffer_size, uint8_t epd_location, bool invert) {
    epd_writeRAMcommand(epd_location);
//    serial_printf("Writing from RAM[0x%x]: \n", &frame_buffer);
//    HAL_Delay(10);
    EPD_DC_HIGH;
//    HAL_Delay(10);
    for (uint32_t i = 0; i < buffer_size; i++) {
        uint8_t d = invert ? ~frame_buffer[i] : frame_buffer[i];
        EPD_CS_LOW;
        HAL_Delay(5);
        HAL_SPI_Transmit(&EPD_SPI, &d, 1, HAL_MAX_DELAY);
        EPD_CS_HIGH;
        HAL_Delay(5);

//        serial_printf("%x ", d);
//        if ((i + 1) % (width / 8) == 0) {
//            serial_printf("\n");
//        }
    }
    EPD_CS_HIGH;
    HAL_Delay(2);
    return;
}

/*!
 * @brief This function writes the current display buffer(s) to the EPD and draws an image.
 * @param sleep     Set to true if the EPD should go to sleep after drawing.
 */
void display(bool sleep) {
    epd_powerUp();
    if (use_sram) {
        write_SRAM_to_epd(buffer1_addr, buffer1_size, 0, false);
    } else {
        write_RAM_to_epd(buffer1, buffer1_size, 0, false);
    }
#ifdef EPD_USE_COLOR
    if (buffer2_size != 0) {
        HAL_Delay(2);
        if (use_sram) {
            write_SRAM_to_epd(buffer2_addr, buffer2_size, 1, false);
        } else {
            write_RAM_to_epd(buffer2, buffer2_size, 1, false);
        }
    }
#endif

    epd_update();
    partials_since_last_full_update = 0;

    if (sleep) {
        epd_powerDown();
    }
}

/*!
 * @brief This function clears the display's buffer(s) and draws an image-free screen.
 */
void clear_display() {
    clear_buffer();
#ifdef EPD_USE_COLOR
    display(false);
#else
    epd_powerUp();
    if (use_sram) {
        write_SRAM_to_epd(buffer1_addr, buffer1_size, 0, false);
    } else {
        write_RAM_to_epd(buffer1, buffer1_size, 0, false);
    }
    HAL_Delay(2);
    if (use_sram) {
        write_SRAM_to_epd(buffer1_addr, buffer1_size, 1, false);
    } else {
        write_RAM_to_epd(buffer1, buffer1_size, 1, false);
    }
    epd_update();
    partials_since_last_full_update = 0;
#endif
    HAL_Delay(100);
}

/*!
 * @brief This function draws a pixel in the display buffer.
 * @param x     The x-coordinate of the pixel
 * @param y     The y-coordinate of the pixel
 * @param color The color of the pixel
 */
void draw_pixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
        serial_printf("Pixel off screen! (%d, %d)", x, y);
        return; // Pixel is off screen, no point in trying to get it to work
    }

    uint8_t *black_pBuf;
#ifdef EPD_USE_COLOR
    uint8_t *color_pBuf;
#endif

    switch (rotation) {
    	case 0:
    		y = y + 8;
    		break;
        case 1: EPD_swap(x, y);
            x = EPD_WIDTH - x - 1;
            break;
        case 2:
            x = EPD_WIDTH - x - 1;
//            y = EPD_HEIGHT - y - 1;
            y = EPD_HEIGHT - y - 1 + 8;
            break;
        case 3: EPD_swap(x, y);
            y = EPD_HEIGHT - y - 1;
            break;
    }

    uint16_t addr = ((uint32_t) (EPD_WIDTH - 1 - x) * (uint32_t) EPD_HEIGHT + y) / 8;
    uint8_t black_c;
#ifdef EPD_USE_COLOR
    uint8_t color_c;
#endif

    if (use_sram) {
        black_c = sram_read8(black_buffer_addr + addr, MCPSRAM_READ);
        black_pBuf = &black_c;
#ifdef EPD_USE_COLOR
        color_c = sram_read8(color_buffer_addr + addr, MCPSRAM_READ);
        color_pBuf = &color_c;
#endif
    } else {
        black_pBuf = black_buffer + addr;
#ifdef EPD_USE_COLOR
        color_pBuf = (color_buffer != NULL) ? color_buffer + addr : NULL;
#endif
    }

#ifdef EPD_USE_COLOR
    bool color_bit;
    color_bit = layer_colors[color] & 0x2;
    if ((color_bit && color_buffer_inverted) || (!color_bit && !color_buffer_inverted)) {
        *color_pBuf &= ~(1 << (7 - y % 8));
    } else {
        *color_pBuf |= (1 << (7 - y % 8));
    }
#endif

    bool black_bit;
    black_bit = layer_colors[color] & 0x1;
    if ((black_bit && black_buffer_inverted) || (!black_bit && !black_buffer_inverted)) {
        *black_pBuf &= ~(1 << (7 - y % 8));
    } else {
        *black_pBuf |= (1 << (7 - y % 8));
    }

    if (use_sram) {
#ifdef EPD_USE_COLOR
        sram_write8(color_buffer_addr + addr, *color_pBuf, MCPSRAM_WRITE);
#endif
        sram_write8(black_buffer_addr + addr, *black_pBuf, MCPSRAM_WRITE);
    }
}
