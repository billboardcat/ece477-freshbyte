#ifndef __SRAM_H__
#define __SRAM_H__

#define MCPSRAM_READ 0x03  ///< read command
#define MCPSRAM_WRITE 0x02 ///< write command
#define MCPSRAM_RDSR 0x05  ///< read status register command
#define MCPSRAM_WRSR 0x01  ///< write status register command

#define K640_SEQUENTIAL_MODE (1 << 6) ///< put ram chip in sequential mode

void sram_init();
void sram_csHigh(void);
void sram_csLow(void);
void sram_write(uint16_t addr, uint8_t* buf, uint16_t num, uint8_t reg);
void sram_read(uint16_t addr, uint8_t *buf, uint16_t num, uint8_t reg);
uint8_t sram_read8(uint16_t addr, uint8_t reg);
uint16_t sram_read16(uint16_t addr);
void sram_write8(uint16_t addr, uint8_t val, uint8_t reg);
void sram_write16(uint16_t addr, uint16_t val) ;
void sram_erase(uint16_t addr, uint16_t length, uint8_t val);

#endif