#ifndef SPIH_H
#define SPIH_H

#include "driver/spi_master.h"
#include <stddef.h>
#include <stdint.h>

void SPI_Setup(void);

// SPI commands (Normal function)
void isoSPI_tx_rx(const uint8_t *command, size_t cmd_length, uint8_t *responses,
                  size_t response_len, spi_device_handle_t spi);

void isoSPI_transmit(const uint8_t *command, size_t data_length,
                     spi_device_handle_t spi);

uint16_t Compute_Command_PEC(const uint8_t *data, size_t data_length);
uint16_t Compute_Data_PEC(const uint8_t *data, size_t data_length);
void adbms_wakeup(spi_device_handle_t dev);
uint16_t pec15_calc(const uint8_t *data, size_t len);

extern spi_device_handle_t spi_cs1, spi_cs2;
extern const uint16_t crc15Table[256];
#endif
