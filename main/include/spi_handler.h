#ifndef SPIH_H
#define SPIH_H

#include "Adbms_handler.h"
#include "driver/spi_master.h"
#include <stddef.h>
#include <stdint.h>

void SPI_Setup(void);

// SPI commands (Normal function)
void adbms_send_command(uint16_t command, spi_device_handle_t spi);
void adbms_fetch_data(uint8_t *responses, size_t response_len,
                      spi_device_handle_t spi);

extern spi_device_handle_t spi_cs1, spi_cs2;

#endif
