#include "main.h"
#include "sram.h"
#include "spi.h"
//#include "serial_print.h"
#include "stm32l0xx_hal.h"
#include <stdint.h>

#define SRAM_SPI hspi1


// Function Code
void sram_csHigh() {
    HAL_GPIO_WritePin(SRAM_CS_GPIO_Port, SRAM_CS_Pin, GPIO_PIN_SET);
}

void sram_csLow() {
    HAL_GPIO_WritePin(SRAM_CS_GPIO_Port, SRAM_CS_Pin, GPIO_PIN_RESET);
}

/*!
    @brief initializes the sram module
*/
void sram_init() {
    sram_csHigh();
    HAL_Delay(100);
    sram_csLow();
    uint8_t magic_value = 0xFF;
    for (int i = 0; i < 3; i++) {
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&SRAM_SPI, &magic_value, 1, HAL_MAX_DELAY);
//        if (status != HAL_OK) {
//            char error[] = "SRAM_INIT: FAILED INIT MAGIC\r\f";
//            serial_println(error);
//        }
    }
    sram_csHigh();
}

/*!
    @param addr the addres to write to
    @param buf the data buffer to write
    @param num the nubmer of bytes to write (from the buffer)
    @param reg pass MCPSRAM_WRSR if you're writing the status register, MCPSRAM_WRITE if you are writing data. Use MCPSRAM_WRITE in Adafruit functions that don't specify reg (default value).
*/
void sram_write(uint16_t addr, uint8_t* buf, uint16_t num, uint8_t reg) {
    sram_csLow();

    // write command and address
    uint8_t cmdbuf[3] = {
        reg,
        (uint8_t) (addr >> 8),
        (uint8_t) (addr & 0xFF)
    };
    
    for (int i = 0; i < 3; i++) {
        uint8_t d = cmdbuf[i];
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&SRAM_SPI, &d, 1, HAL_MAX_DELAY);
//        if (status != HAL_OK) {
//            char error[] = "SRAM_WRITE: FAILED TO SEND CMD/ADDR\r\f";
//            serial_println(error);
//        }
        if (reg != MCPSRAM_WRITE) {
            break;
        }
    }

    for (int i = 0; i < num; i++) {
        uint8_t d = buf[i];
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&SRAM_SPI, &d, 1, HAL_MAX_DELAY);
//        if (status != HAL_OK) {
//            char error[] = "SRAM_WRITE: FAILED TO SEND\r\f";
//            serial_println(error);
//        }
    }

    sram_csHigh();
    HAL_Delay(100);
}

/*!
    @param addr the address to read from
    @param buf the data buffer to read into
    @param num the number of bytes to read
    @param reg pass MCPSRAM_RDSR if you're reading the status register, MCPSRAM_READ if you are reading data. Use MCPSRAM_READ in Adafruit functions that don't specify reg (default value).
*/
void sram_read(uint16_t addr, uint8_t *buf, uint16_t num, uint8_t reg) {
    sram_csLow();

    // read command and address
    uint8_t cmdbuf[3] = {
        reg,
        (uint8_t) (addr >> 8),
        (uint8_t) (addr & 0xFF)
    };

    for (int i = 0; i < 3; i++) {
        uint8_t d = cmdbuf[i];
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&SRAM_SPI, &d, 1, HAL_MAX_DELAY);
//        if (status != HAL_OK) {
//            char error[] = "SRAM_READ: FAILED TO SEND";
//            serial_println(error);
//        }
        if (reg != MCPSRAM_READ) {
            break;
        }
    }

    for (int i = 0; i < num; i++) {
        HAL_StatusTypeDef status = HAL_SPI_Receive(&SRAM_SPI, buf++, 1, HAL_MAX_DELAY);
//        if (status != HAL_OK) {
//            char *msg;
//            switch (status) {
//                case HAL_ERROR:
//                    msg = "SRAM_READ: FAILED TO READ - HAL_ERROR";
//                    serial_println(msg);
//                    break;
//                case HAL_BUSY:
//                    msg = "SRAM_READ: FAILED TO READ - HAL_BUSY";
//                    serial_println(msg);
//                    break;
//                case HAL_TIMEOUT:
//                    msg = "SRAM_READ: FAILED TO READ - HAL_TIMEOUT";
//                    serial_println(msg);
//                    break;
//                default:
//                    msg = "SRAM_READ: FAILED TO READ - UNKNOWN ERROR";
//                    serial_println(msg);
//                    break;
//            }
//        }
    }
    sram_csHigh();
}

/*! @brief read 1 byte of data at the specified address
    @param addr the address to read data at
    @param reg pass MCPSRAM_RDSR if you're reading the status register, MCPSRAM_READ if you are reading data. Use MCPSRAM_READ in Adafruit functions that don't specify reg (default value).
    @returns the read data byte
*/
uint8_t sram_read8(uint16_t addr, uint8_t reg) {
    uint8_t c;
    sram_read(addr, &c, 1, reg);
    return c;
}

/*!
    @param addr the address to read
    @returns the read data bytes as a 16 bit unsigned integer
*/
uint16_t sram_read16(uint16_t addr) {
    uint8_t b[2];
    sram_read(addr, b, 2, MCPSRAM_READ);
    return (uint16_t) ((b[0] << 8) | b[1]);
}

/*!
    @param addr the address to write to
    @param val the value to write
    @param reg MCPSRAM_WRITE if writing data, MCPSRAM_WRSR if writing a status register. Use MCPSRAM_WRITE in Adafruit functions that don't specify reg (default value).
*/
void sram_write8(uint16_t addr, uint8_t val, uint8_t reg) {
    sram_write(addr, &val, 1, reg);
}

/*!
    @param addr the address to write to
    @param val the value to write
*/
void sram_write16(uint16_t addr, uint16_t val) {
    uint8_t buf[2] = {(uint8_t) (val >> 8), (uint8_t) (val & 0xFF)};
    sram_write(addr, buf, 2, MCPSRAM_WRITE);
}

/*!
    @param addr the address to start the erase at
    @param length the number of byts to fill
    @param val the value to set the data to
*/
void sram_erase(uint16_t addr, uint16_t length, uint8_t val) {
    sram_csLow();

    // write command and addrress
    uint8_t cmdbuf[3] = {
        MCPSRAM_WRITE,
        (uint8_t) (addr >> 8),
        (uint8_t) (addr & 0xFF)
    };

    for (int i = 0; i < 3; i++) {
        uint8_t d = cmdbuf[i];
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&SRAM_SPI, &d, 1, HAL_MAX_DELAY);
//        if (status != HAL_OK) {
//            char error[] = "SRAM_ERASE: TX ERR\r\f";
//            serial_println(error);
//        }
    }

    // write buffer of data
    for (int i = 0; i < length; i++) {
        uint8_t d = val;
        HAL_StatusTypeDef status = HAL_SPI_Transmit(&SRAM_SPI, &d, 1, HAL_MAX_DELAY);
//        if (status != HAL_OK) {
//            char error[] = "SRAM_ERASE: TX ERR\r\f";
//            serial_println(error);
//        }
    }

    sram_csHigh();
}