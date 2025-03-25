#include "include/spi_handler.h"
#include "Robin_handler.h"
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

#define SPI_HOST_D SPI2_HOST
#define MISO_NUM 12
#define MOSI_NUM 13
#define CLK_NUM 14
#define SPI_CS1 46
#define SPI_CS2 11

#define DATA_PEC_POLY 0x22D
#define DATA_PEC_INIT 0x0008
#define COMMAND_PEC_POLY 0b110001011001101
#define COMMAND_PEC_INIT 0b0110001011001101;

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
      .clock_speed_hz = 1 * 1000 * 1000, // 1MHz
      .mode = 0,
      .spics_io_num = SPI_CS1,
      .queue_size = 7,
      .flags = SPI_DEVICE_HALFDUPLEX};

  // Add device to registry
  spi_device_interface_config_t devcf_receiver_CS2 = {
      .clock_speed_hz = 1 * 1000 * 1000, // 1MHz
      .mode = 0,
      .spics_io_num = SPI_CS2,
      .queue_size = 7,
      .flags = SPI_DEVICE_HALFDUPLEX};

  ESP_ERROR_CHECK(spi_bus_add_device(SPI_HOST_D, &devcf_sender_CS1, &spi_cs1));
  ESP_ERROR_CHECK(
      spi_bus_add_device(SPI_HOST_D, &devcf_receiver_CS2, &spi_cs2));

  ESP_LOGI(TAG, "SPI Sender and Receiver are Initilialised");
}

/*
 * How does PEC work??
 *
 *
 */
// The Data PEC is a 10 bit Polynomial Error Check placed after every register
// data packet.
uint16_t Compute_Data_PEC(const uint8_t *data, size_t response_len) {
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

  return remainder;
}

// The Command PEC is a 15 bit Polynomial Error Check placed after the CMD0 and
// CMD1 in the data packet.
uint16_t Compute_Command_PEC(uint8_t *data, size_t data_length) {
  // Polynomial for X^15 + X^14 + X^10 + X^8 + X^7 + X^4 + X^3 + 1
  // For example at Position 15, 14, 10, 8, 7, 4, 3 and 0, bit = 1
  // 0110001011001101 or
  // Stored in 16 Bits but masked to 15 Bits
  const uint16_t Poly_15bit = COMMAND_PEC_POLY;

  // Initial Remainder = Seed at the Start
  // It shall be placed at the bottom of a 16 Bit value
  uint16_t remainder = COMMAND_PEC_INIT;

  // Process each byte
  for (size_t i = 0; i < data_length; i++) {

    // XOR the incoming byte into the 'top' of the remainder
    // Beacause we use only 15 bits, we shift the bits up by one
    remainder ^= (data[i] << 1);

    // Now process
    for (int bit = 0; bit < 8; bit++) {

      // Test if top bit is set
      if (remainder & 0x200) { // 0x200 = (1 << 9)

        // Left Shift, then XOR with Polynomial
        remainder ^= (uint16_t)(remainder << 1) ^ Poly_15bit;
      } else {
        remainder <<= 1;
      }

      // Always mask back to 15 bits so prevent overflow
      remainder &= 0x7FFF; // 15 bits 0b0111111111111111
    }
  }
  return remainder;
}

// Extraction of PEC from the received data
/* uint16_t extract_received_pec(uint16_t *rxbuffer) {} */

// Transmission of
void isoSPI_transmit(uint8_t *cmd_tx, size_t cmd_length,
                     spi_device_handle_t spi) {

  // (16-bit command + 15-bit PEC)
  spi_transaction_t t_tx = {.length = cmd_length * 8,
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

  // length should be size per module * Total_num of Module * 8 Bits
  spi_transaction_t t_rx = {.length = response_len,
                            .tx_buffer = NULL,
                            .rxlength = response_len,
                            .rx_buffer = responses};

  esp_err_t err = spi_device_transmit(spi_cs2, &t_rx);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to Transfer Command");
  }
}
