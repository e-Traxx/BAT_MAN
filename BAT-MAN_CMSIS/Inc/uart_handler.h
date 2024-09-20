// Include
#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
 extern "C" {
#endif

// Includes 
#include <stdint.h>



void UART_INIT(void);
void USART_Tx_Data(unsigned char x);
int U_debug(char Tx[]);
void USART3_IRQHandler(void);
uint8_t USART_receive(void);
#ifdef __cplusplus
}
#endif

#endif
