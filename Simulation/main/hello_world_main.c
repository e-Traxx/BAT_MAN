#include "driver/spi_slave.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SPI_HOST SPI2_HOST
#define SPI_MOSI 10
#define SPI_MISO 9
#define SPI_CLK 8
#define SPI_CS 7

#define TOTAL_MODULES 14
#define CELL_COUNT 10

static const char *TAG = "ADBMS6830_SIM";

// Simulated data storage
uint16_t simulated_cell_voltages[TOTAL_MODULES][CELL_COUNT];
uint16_t simulated_gpio_adc[TOTAL_MODULES][10];
uint8_t simulated_status[TOTAL_MODULES][6];

volatile bool conversion_complete = false;

// Generate random float in range
float random_float(float min, float max) {
  return min + ((float)rand() / RAND_MAX) * (max - min);
}

// Simulate cell voltages (3.00V - 4.12V) and GPIO ADC values (0.5V - 3.6V)
void generate_mock_data() {
  for (int i = 0; i < TOTAL_MODULES; i++) {
    for (int j = 0; j < CELL_COUNT; j++) {
      float voltage = random_float(3.00, 4.12);
      simulated_cell_voltages[i][j] = (uint16_t)(voltage * 1000);
    }
    for (int j = 0; j < 10; j++) {
      float adc_value = random_float(0.5, 3.6);
      simulated_gpio_adc[i][j] = (uint16_t)((adc_value / 5.0) * 65535);
    }
    memset(simulated_status[i], 0xAB,
           sizeof(simulated_status[i])); // Dummy status
  }
}

// Simulate ADC Conversion Time
void start_conversion() {
  conversion_complete = false;
  vTaskDelay(pdMS_TO_TICKS(2)); // Simulated 2ms delay
  conversion_complete = true;
}

// Handle incoming SPI commands
void handle_spi_command(uint8_t *cmd, uint8_t *response, size_t length) {
  uint8_t command = cmd[0];

  switch (command) {
  case 0x01: // ADCV - Start Cell Voltage Conversion
    ESP_LOGI(TAG, "ADCV Command Received - Starting Conversion...");
    start_conversion();
    memset(response, 0, length); // ADCV does not return data
    break;

  case 0x04: // ADAX2 - Return GPIO ADC (Temperature Simulation)
    ESP_LOGI(TAG, "ADAX2 Command Received");
    if (!conversion_complete) {
      ESP_LOGW(TAG, "Conversion not completed yet!");
      memset(response, 0xFF, length); // Indicate conversion not done
      break;
    }
    for (int i = 0; i < TOTAL_MODULES; i++) {
      for (int j = 0; j < 10; j++) {
        response[i * 10 * 2 + j * 2] = (simulated_gpio_adc[i][j] >> 8) & 0xFF;
        response[i * 10 * 2 + j * 2 + 1] = simulated_gpio_adc[i][j] & 0xFF;
      }
    }
    break;

  case 0x02: // STATC - Return Status Register Data
    ESP_LOGI(TAG, "STATC Command Received");
    for (int i = 0; i < TOTAL_MODULES; i++) {
      memcpy(&response[i * 6], simulated_status[i], 6);
    }
    break;

  case 0x10: // RDCVA - Read Cell Voltages (Bank A)
  case 0x11: // RDCVB - Read Cell Voltages (Bank B)
  case 0x12: // RDCVC - Read Cell Voltages (Bank C)
    if (!conversion_complete) {
      ESP_LOGW(TAG, "Voltage Read Requested but Conversion Not Done!");
      memset(response, 0xFF, length); // Indicate conversion not ready
      break;
    }
    ESP_LOGI(TAG, "Reading Cell Voltages for Bank: 0x%02X", command);
    for (int i = 0; i < TOTAL_MODULES; i++) {
      for (int j = 0; j < CELL_COUNT; j++) {
        response[i * CELL_COUNT * 2 + j * 2] =
            (simulated_cell_voltages[i][j] >> 8) & 0xFF;
        response[i * CELL_COUNT * 2 + j * 2 + 1] =
            simulated_cell_voltages[i][j] & 0xFF;
      }
    }
    break;

  default:
    ESP_LOGW(TAG, "Unknown Command Received: 0x%02X", command);
    memset(response, 0xFF, length); // Send invalid data on unknown command
    break;
  }
}

// SPI slave task
void spi_slave_task(void *arg) {
  uint8_t recv_buffer[4] = {0};
  uint8_t send_buffer[300] = {0};

  spi_slave_transaction_t trans = {.length = sizeof(recv_buffer) * 8,
                                   .tx_buffer = send_buffer,
                                   .rx_buffer = recv_buffer};

  while (1) {
    if (spi_slave_transmit(SPI_HOST, &trans, portMAX_DELAY) == ESP_OK) {
      handle_spi_command(recv_buffer, send_buffer, sizeof(send_buffer));
    }
  }
}

// Initialize SPI slave
void init_spi_slave() {
  spi_bus_config_t buscfg = {.mosi_io_num = SPI_MOSI,
                             .miso_io_num = SPI_MISO,
                             .sclk_io_num = SPI_CLK,
                             .quadwp_io_num = -1,
                             .quadhd_io_num = -1};

  spi_slave_interface_config_t slvcfg = {
      .mode = 0, .spics_io_num = SPI_CS, .queue_size = 1, .flags = 0};

  ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
  ESP_ERROR_CHECK(
      spi_slave_initialize(SPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));
}

void app_main() {
  ESP_LOGI(TAG, "Starting ADBMS6830 Simulator...");
  generate_mock_data();
  init_spi_slave();
  xTaskCreate(spi_slave_task, "spi_slave_task", 4096, NULL, 5, NULL);
}
