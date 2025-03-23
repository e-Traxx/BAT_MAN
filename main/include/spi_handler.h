#ifndef SPIH_H
#define SPIH_H

#include "Adbms_handler.h"
#include "driver/spi_master.h"
#include <stddef.h>
#include <stdint.h>

void SPI_Setup(void);

// SPI commands (Normal function)
void isoSPI_transmit(uint8_t *command, size_t data_length,
                     spi_device_handle_t spi);
void isoSPI_receive(uint8_t *responses, size_t response_len,
                    spi_device_handle_t spi);

uint16_t Compute_Command_PEC(uint8_t *data, size_t data_length);
uint16_t Compute_Data_PEC(const uint8_t *data, size_t data_length);

extern spi_device_handle_t spi_cs1, spi_cs2;

#endif
