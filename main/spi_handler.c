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

#define DATA_PEC_POLY 0x22D
#define DATA_PEC_INIT 0x0008

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
/* uint16_t calculate_PEC(const uint16_t *data, size_t len) { */
/*   uint8_t crc = 0x41; */
/*   for (size_t i = 0; i < len; i++) { */
/*     // XOR each byte with current CRC */
/*     crc ^= data[i]; */
/*     for (uint8_t bit = 0; bit < 8; bit++) { */
/*       if (crc & 0x80) {          // if MSB is 1 */
/*         crc = (crc << 1) ^ 0x70; // shift and XOR with Polynomial */
/*       } else { */
/*         crc <<= 1; // shift */
/*       } */
/*     } */
/*   } */
/**/
/*   return crc; */
/* } */

uint16_t Compute_Data_Pec(const uint8_t *data, size_t response_len) {
  // Remainder uses bits [9:0]. We will store it in a 16-bit variable
  uint16_t remainder = DATA_PEC_INIT;

  // Process all Bytes
  for (size_t i = 0; i < response_len; i++) {
    for (uint8_t bitPos = 0; bitPos < 8; bitPos++) {

      // Next Data bit from data (LSB first)
      uint8_t dataBit = (data[i] >> bitPos) & 0x01;

      // Current LSB of Remainder
      uint8_t crcBit = remainder & 0x01;

      // shift remainder by 1
      remainder >>= 1;

      // Xor remain with the Polynomial if
      if ((dataBit ^ crcBit) != 0) {
        remainder ^= DATA_PEC_POLY;
      }
    }
  }

  // final Remainder is now in bits [9:0]
  remainder &= 0x03FF;
  return remainder;
}

size_t Prepare_Data_write_buf(const uint8_t *userData, size_t dataLen,
                              uint8_t *outBUF) {
  // First Copy user data
  memcpy(outBUF, userData, dataLen);

  // Compute 10-Bit PEC
  uint16_t remainder = Compute_Data_Pec(userData, dataLen);

  // remainder has 10 bit [9...0]. We place them into [15...6] of two Bytes:
  //   outBuf[dataLen]   = remainder[9..2]
  //   outBuf[dataLen+1] = remainder[1..0] << 6
  //
  // (We keep bits [5..0] = 0 in the final byte.)

  outBUF[dataLen] = (uint8_t)((remainder >> 2) & 0xFF); // bits [9..2]
  outBUF[dataLen + 1] =
      (uint8_t)((remainder & 0x3) << 6); // bits [1..0] in [7..6]

  return (dataLen + 2);
}
}

void isoSPI_transmit(uint16_t cmd_tx, spi_device_handle_t spi) {

  // (16-bit command + 15-bit PEC)
  spi_transaction_t t_tx = {.length = 24, // Sending 3 Bytes
                            .tx_buffer = &cmd_tx,
                            .rxlength = 0,
                            .rx_buffer = NULL};

  esp_err_t err = spi_device_transmit(spi_cs1, &t_tx);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to Transfer Command");
  }
}

void isoSPI_receive(uint8_t *responses, size_t response_len,
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
