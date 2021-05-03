#include "serial_print.h"
#include "usart.h"
#include <stdint.h>
#include <stdarg.h>

#define SERAL_UART huart2

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

void serial_clear() {
  char clear_sequence[] = {0x1b, 0x5B, 0x32, 0x4A, 0x1b, 0x5B, 0x48};
  HAL_UART_Transmit(&SERAL_UART, (unsigned char*) clear_sequence, sizeof clear_sequence, HAL_MAX_DELAY);
}

void serial_print(char string[]) {
  HAL_UART_Transmit(&SERAL_UART, (unsigned char*) string, strlen(string), HAL_MAX_DELAY);
}

void serial_println(char *string) {
  HAL_UART_Transmit(&SERAL_UART, (unsigned char*) string, strlen(string), HAL_MAX_DELAY);
  char newline[] = "\r\f";
  HAL_UART_Transmit(&SERAL_UART, (unsigned char*) newline, 2, HAL_MAX_DELAY);
}

void serial_putc(char c) {
    HAL_UART_Transmit(&SERAL_UART, (unsigned char*) &c, 1, HAL_MAX_DELAY);
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
            	serial_print("\r\f");
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
		}	
	} 

	//Module 3: Closing argument list to necessary clean-up
	va_end(arg); 
} 
 
