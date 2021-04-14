#include "serial_print.h"
#include "usart.h"
#include <stdarg.h>

#define WIFI_UART huart1
#define SERIAL_UART huart2

enum uart_line_t current_line;
UART_HandleTypeDef serial_line;

// Internal functions to serial_print
char *convert(unsigned int num, int base);
size_t strlen(const char *str);

char *convert(unsigned int num, int base) { 
	static char Representation[]= "0123456789ABCDEF";
	static char buffer[50]; 
	char *ptr; 
	
	ptr = &buffer[49]; 
	*ptr = '\0'; 
	
	do { 
		*--ptr = Representation[num%base]; 
		num /= base; 
	} while(num != 0); 
	
	return(ptr); 
}

/*!
    @brief https://stackoverflow.com/questions/22520413/c-strlen-implementation-in-one-line-of-code
*/
size_t strlen (const char *str) {
    return (*str) ? strlen(++str) + 1 : 0;
}
// End of internal functions for serial_print

void serial_select(enum uart_line_t line) {
    switch (line) {
        case WIFI:
            current_line = line;
            serial_line = WIFI_UART;
            break;
        case DEBUG_PRINT:
            current_line = line;
            serial_line = SERIAL_UART;
            break;
    }
}

void serial_clear() {
  char clear_sequence[] = {0x1b, 0x5B, 0x32, 0x4A, 0x1b, 0x5B, 0x48};
  HAL_UART_Transmit(&serial_line, (unsigned char*) clear_sequence, sizeof clear_sequence, HAL_MAX_DELAY);
}

void serial_print(char string[]) {
  HAL_UART_Transmit(&serial_line, (unsigned char*) string, strlen(string), HAL_MAX_DELAY);
}

void serial_println(char *string) {
  HAL_UART_Transmit(&serial_line, (unsigned char*) string, strlen(string), HAL_MAX_DELAY);
  char* newline = ((current_line == WIFI) ? "\r\n" : "\r\f");
  HAL_UART_Transmit(&serial_line, (unsigned char*) newline, 2, HAL_MAX_DELAY);
}

void serial_putc(char c) {
    HAL_UART_Transmit(&serial_line, (unsigned char*) &c, 1, HAL_MAX_DELAY);
}

void serial_receive(char * str, int length) {
  HAL_UART_Receive_DMA(&serial_line, str, length);
}

/*!
    @brief a version of printf implmented from http://www.firmcodes.com/write-printf-function-c/
*/
void serial_printf(char format[], ...) { 
	char *traverse; 
	unsigned int i;
	int signed_i;
    char c;
	char *s; 
	
	//Module 1: Initializing Myprintf's arguments 
	va_list arg; 
	va_start(arg, format); 
	
	for(traverse = format; *traverse != '\0'; traverse++) { 
		while( (*traverse != '%') && (*traverse != '\0')) {
            if (*traverse == '\n') {
            	serial_print(((current_line == WIFI) ? "\r\n" : "\r\f"));
            }
            else {
            	serial_putc(*traverse);
            }
			traverse++; 
		} 

		if (*traverse == '\0') break;
		traverse++; 
		
		//Module 2: Fetching and executing arguments
		switch(*traverse) { 
			case 'c' : 
                c = va_arg(arg,int);		//Fetch char argument
				serial_putc(c);
				break; 	
			case 'd' : 
                signed_i = va_arg(arg,int); 		//Fetch Decimal/Integer argument
				if (signed_i < 0) {
                    signed_i = -signed_i;
                    serial_print("-");
                } 
                serial_print(convert(signed_i, 10));
                break; 	
			case 'o':
                i = va_arg(arg,unsigned int); //Fetch Octal representation
                serial_print(convert(i, 8));
                break; 
			case 's': 
                s = va_arg(arg,char *); 		//Fetch string
                serial_print(s);
                break; 
			case 'x': 
                i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
                serial_print(convert(i, 16));
                break;

      case '%':
                serial_print("%");
                break;
		}	
	} 

	//Module 3: Closing argument list to necessary clean-up
	va_end(arg); 
} 
 
