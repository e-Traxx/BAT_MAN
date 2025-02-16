#include "include/spi_handler.h"
#include "Adbms_handler.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hal/spi_types.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define SPI_HOST SPI2_HOST
#define MISO_NUM 12
#define MOSI_NUM 13
#define CLK_NUM 14
#define SPI_CS1 46
#define SPI_CS2 11

static const char *TAG = "SPI";
spi_device_handle_t spi_cs1, spi_cs2;


void SPI_Setup(void) {
  esp_err_t ret;

  spi_bus_config_t bus_conf = {.miso_io_num = MISO_NUM,
                               .mosi_io_num = MOSI_NUM,
                               .sclk_io_num = CLK_NUM,
                               .quadwp_io_num = -1,
                               .quadhd_io_num = -1,
                               .max_transfer_sz =
                                   4096}; // for 2 rounds of 10 Modules

  // Initilialise SPI bus
  ret = spi_bus_initialize(HSPI_HOST, &bus_conf, SPI_DMA_CH_AUTO);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "[-] Failed to Initilialise SPI\n");
  }

  // Add device to registry
  spi_device_interface_config_t devcf_sender_CS1 = {
      .clock_speed_hz = 1 * 1000 * 1000, // 2MHz
      .mode = 0,
      .spics_io_num = SPI_CS1,
      .queue_size = 7,
      .flags = SPI_DEVICE_HALFDUPLEX};

  // Add device to registry
  spi_device_interface_config_t devcf_receiver_CS2 = {
      .clock_speed_hz = 1 * 1000 * 1000, // 2MHz
      .mode = 0,
      .spics_io_num = SPI_CS2,
      .queue_size = 7,
      .flags = SPI_DEVICE_HALFDUPLEX};

  ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST, &devcf_sender_CS1, &spi_cs1));
  ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST, &devcf_receiver_CS2, &spi_cs2));

  ESP_LOGI(TAG, "SPI Sender and Receiver are Initilialised");
}

/*
 * How does PEC work??
 *
 * */
uint8_t calculate_PEC(const uint8_t *data, size_t len) {
  uint8_t crc = 0x41;
  for (size_t i = 0; i < len; i++) {
    // XOR each byte with current CRC
    crc ^= data[i];
    for (uint8_t bit = 0; bit < 8; bit++) {
      if (crc & 0x80) {          // if MSB is 1
        crc = (crc << 1) ^ 0x70; // shift and XOR with Polynomial
      } else {
        crc <<= 1; // shift
      }
    }
  }

  return crc;
}

void adbms_send_command(uint16_t command, spi_device_handle_t spi) {
  uint8_t cmd_tx[3] = {(command >> 3) & 0xFF, command & 0xFF, 0xFF};
  cmd_tx[2] = calculate_PEC(cmd_tx, 2); // Calculate PEC

  // (16-bit command + 8-bit PEC)
  spi_transaction_t t_tx = {.length = 24, // Sending 3 Bytes
                            .tx_buffer = cmd_tx,
                            .rxlength = 0,
                            .rx_buffer = NULL};

  esp_err_t err = spi_device_transmit(spi_cs1, &t_tx);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to Transfer Command");
  }
}

void adbms_fetch_data(uint8_t *responses, size_t response_len,
                      spi_device_handle_t spi) {

  ESP_LOGI(TAG, "Reading Cell Voltages");
  spi_transaction_t t_rx = {.length = response_len * 8,
                            .tx_buffer = NULL,
                            .rxlength = response_len * 8,
                            .rx_buffer = responses};

  esp_err_t err = spi_device_transmit(spi_cs2, &t_rx);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to Transfer Command");
  }
}
