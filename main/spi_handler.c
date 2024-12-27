#include "include/spi_handler.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "hal/spi_types.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

#define MISO_NUM 12
#define MOSI_NUM 13
#define CLK_NUM 14

static const char *TAG = "SPI setup";

uint8_t ADCV_command[] = {0x02, 0xC0, 0x0};
spi_device_handle_t spi;

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
  ret = spi_bus_initialize(HSPI_HOST, &bus_conf, 1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "[-] Failed to Initilialise SPI\n");
  }

  // Add device to registry
  spi_device_interface_config_t dev_conf = {.clock_speed_hz =
                                                1 * 1000 * 1000, // 1MHz
                                            .mode = 0,
                                            .queue_size = 7,
                                            .flags = SPI_DEVICE_HALFDUPLEX};

  ret = spi_bus_add_device(HSPI_HOST, &dev_conf, &spi);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "[-] Failed to add Device %s", esp_err_to_name(ret));
  }
}

/*
 *
 * The Slaves work on command basis, they will only respond to commands sent by
 * the Master, This System uses the oneshot query system, where the master asks
 * for values and the values are provided
 *  //! Continous measurements are avoided, lieber !!
 *
 *
 *  ! This reads all filtered values.
 *  RDFCALL command  : 0 0 0 0 0 0 0 1 1 0 0
 *
 *  ! This reads all the AUX values (Temperature values).
 *  RDASALL: 0 0 0 0 0 1 1 0 1 0 1
 *
 *
 */

/* // Template for the real deal */
/* void spi_ADBMS_ADCV_command(size_t cmd_len, uint8_t *cmd, uint8_t *response,
 */
/*                             size_t resp_len) { */
/*   spi_transaction_t adcv_command = {.flags = SPI_TRANS_USE_TXDATA | */
/*                                              SPI_TRANS_USE_RXDATA, */
/*                                     .length = cmd_len * 8, */
/*                                     .rxlength = resp_len * 8, */
/*                                     .tx_buffer = cmd, */
/*                                     .rx_buffer = response}; */
/* } */
/**/

/*
 *
 * The Slaves modules are comprised of 14 Modules sending 10 values back to the
 * Master Each value consists of 16 bits (2 Bytes) per cell voltage value. The
 * total amount of Bytes per module is 10 x 2 Bytes = 20 Bytes The Total amount
 * of Bytes received from the whole slave system amount to 20 x 14 = 280 Bytes
 *   The temperature measurements are different: they use 12 Bits per Value
 *                       12 bits x 10 = 120 Bits
 *                       14 x 120 bits = 1.680 Bits (2^12 = 4096 Bits)
 *
 */
void read_all_modules(uint8_t *response, size_t resp_len) {

  spi_transaction_t trans = {.length = resp_len * 8,
                             .rx_buffer = response,
                             .flags = 0,
                             .tx_buffer = NULL,
                             .rxlength = resp_len * 8};

  if (spi_device_transmit(spi, &trans) != ESP_OK) {
    ESP_LOGI(TAG, "[-] Failed to read modules");
  }
}
