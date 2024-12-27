#include "include/spi_handler.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hal/spi_types.h"
#include "string.h"
#include <stdint.h>

#define MISO_NUM 12
#define MOSI_NUM 13
#define CLK_NUM 14

static const char *TAG = "SPI setup";

spi_device_handle_t spi;

void SPI_Setup(void) {
  esp_err_t ret;

  spi_bus_config_t bus_conf = {.miso_io_num = MISO_NUM,
                               .mosi_io_num = MOSI_NUM,
                               .sclk_io_num = CLK_NUM,
                               .quadwp_io_num = -1,
                               .quadhd_io_num = -1,
                               .max_transfer_sz = 4096};

  // Initilialise SPI bus
  ret = spi_bus_initialize(VSPI_HOST, &bus_conf, 1);
  if (ret != ESP_OK) {
    printf("[-] Failed to Initilialise SPI\n");
  }
}

// Add SPI device to configuration
void spi_add_device() {
  esp_err_t ret;

  spi_device_interface_config_t dev_conf = {.clock_speed_hz =
                                                1 * 1000 * 1000, // 1MHz
                                            .mode = 0,
                                            .queue_size = 7,
                                            .flags = SPI_DEVICE_HALFDUPLEX};

  ret = spi_bus_add_device(HSPI_HOST, &dev_conf, &spi);
  if (ret != ESP_OK) {
    printf("[-] Failed to add Device");
  }
}

// Template for the real deal
void spi_ADBMS_ADCV_command(size_t cmd_len, uint8_t *cmd, uint8_t *response,
                            size_t resp_len) {
  spi_transaction_t adcv_command = {.flags = SPI_TRANS_USE_TXDATA |
                                             SPI_TRANS_USE_RXDATA,
                                    .length = cmd_len * 8,
                                    .rxlength = resp_len * 8,
                                    .tx_buffer = cmd,
                                    .rx_buffer = response};
}

void read_cell_voltages(spi_device_handle_t spi) {
  uint8_t cmd[] = {0x01, 0xC0, 0xFF}; // Example ADCV command
  uint8_t response[35] = {0};         // 16 cells * 2 bytes + PEC

  spi_ADBMS_ADCV_command(*cmd, sizeof(cmd), response, sizeof(response));

  // Parse received data
  uint16_t cell_voltages[16];
  for (int i = 0; i < 16; i++) {
    cell_voltages[i] = (response[2 * i] << 8) | response[2 * i + 1];
  }

  // Format and display the voltages
  for (int i = 0; i < 16; i++) {
    float voltage = cell_voltages[i] * 0.00015; // 150 µV per LSB
    ESP_LOGI(TAG, "Cell %d: %.3f V", i + 1, voltage);
  }
}

// for read test cases
esp_err_t spi_read_data() {
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));

  uint8_t data;
  // Not gonna send request command
  memset(&t, 0, sizeof(t));
  t.length = 8 * 32;
  t.tx_buffer = NULL;
  t.rx_buffer = &data;

  ret = spi_device_transmit(spi, &t);
  if (ret != ESP_OK) {
    printf("[-] Failed to read data %s \n", esp_err_to_name(ret));
  }

  printf("data; %" PRIu8 "\n", data);

  return ESP_OK;
}
