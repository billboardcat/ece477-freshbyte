#ifndef __SERIAL_PRINT_H__
#define __SERIAL_PRINT_H__

void serial_clear(void);
void serial_putc(char c);
void serial_print(char string[]);
void serial_println(char *string);
void serial_printf(char format[], ...);

#endif