#ifndef SPIH_H
#define SPIH_H

#include "esp_err.h"

void SPI_Setup(void);
void spi_add_device(void);

esp_err_t spi_read_data(void);

#endif
