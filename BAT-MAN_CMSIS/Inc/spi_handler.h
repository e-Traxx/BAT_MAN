#ifndef __SPIH_H
#define __SPIH_H

#include <stdint.h>
#ifdef __cplusplus
 extern "C" {
#endif

// Includes 
void SPI_INIT(void);
void SPI1_IRQHandler(void);
void SPI1_Transmit(uint8_t data);
uint8_t SPI1_Receive(void);
uint8_t SPI1_TransmitReceive(uint8_t data);



#ifdef __cplusplus
}
#endif

#endif