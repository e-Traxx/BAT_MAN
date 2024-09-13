// Include
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
 extern "C" {
#endif

// Includes 
#include <stdint.h>


void USART_Tx_Data(unsigned char x);
void UART_INIT(void);

int U_debug(char Tx[]);

#ifdef __cplusplus
}
#endif

#endif
