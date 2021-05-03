#ifndef __SERIAL_PRINT_H__
#define __SERIAL_PRINT_H__

enum uart_line_t {
    WIFI,
    DEBUG_PRINT
};

void serial_select(enum uart_line_t);
void serial_clear(void);
void serial_putc(char c);
void serial_print(char string[]);
void serial_println(char *string);
void serial_printf(char format[], ...);
void serial_receive(char * str, int length);

#endif