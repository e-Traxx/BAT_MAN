#ifndef SPIH_H
#define SPIH_H

#include <stddef.h>
#include <stdint.h>

void SPI_Setup(void);
void spi_add_device(void);

void read_all_modules(uint8_t *response, size_t resp_len);

#endif
