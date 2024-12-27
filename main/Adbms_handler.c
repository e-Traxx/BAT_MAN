#include "Adbms_handler.h"
#include "assert.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "include/can_handler.h"
#include "include/spi_handler.h"
#include "string.h"
#include <stdint.h>

SemaphoreHandle_t adbms_semaphore;
TimerHandle_t adbms_timer;

static const char *TAG = "ADBMS_Query";

void Adbms_query_callback(TimerHandle_t adbms_timer);
void Robin_query(void *args);
void System_report_to_user(Robin_container_t *robin);
void individual_voltages_formatter(individual_voltages_frame_t *frame,
                                   uint16_t voltages[5], uint8_t mux);
void individual_temperatures_formatter(individual_temperatures_frame_t *frame,
                                       uint16_t temps[5], uint8_t mux);

// post Query value updates
void parse_voltages(uint8_t *response, size_t num_modules);

void ADBMS_Setup(void) {
  // Semaphore Setup
  adbms_semaphore = xSemaphoreCreateBinary();
  if (adbms_semaphore == NULL) {
    ESP_LOGE(TAG, "[-] Failed to initialise ADBMS Semaphore");
    return;
  }

  // Create Timer
  adbms_timer = xTimerCreate("ADBMS_Query_timer", pdMS_TO_TICKS(1000), pdTRUE,
                             (void *)0, Adbms_query_callback);
  if (adbms_timer == NULL) {
    ESP_LOGE(TAG, "[-] Failed to initialise ADBMS Timer");
    return;
  }
}

// Gives semaphore to query task every 200ms
void Adbms_query_callback(TimerHandle_t timer) {
  BaseType_t HighPriorityTaskWoken = pdFALSE;
  esp_err_t ret;

  // Give Semaphore to Query task
  ret = xSemaphoreGiveFromISR(adbms_semaphore, &HighPriorityTaskWoken);
  if (ret != pdPASS) {
    ESP_LOGE(TAG, "[-] %s Failed to give Semaphore to ADBMS query",
             esp_err_to_name(ret));
  }
}

/*
 *
 * Communication Side
 *
 * SPI -> Communication with 14 Slave modules
 *     * Report Cell voltage Values
 *     * Report Temperature Values
 *
 * CAN -> Communication with ECU and User laptop as well as Charger
 *     * Report individual_voltages and temperature to User
 *     * Report Diagnostic data to ECU for latching and display
 *
 */

// Query the Slave modules
void Robin_query(void *args) {

  while (1) {
    if (xSemaphoreTake(adbms_semaphore, portMAX_DELAY) == pdTRUE) {
      Robin_container_t *robin = malloc(sizeof(Robin_container_t));

      ESP_LOGV(TAG, "[+] Querying... \n");
      // First query the Cells
      uint8_t response[1024] = {0}; // response container
      read_all_modules(response, sizeof(response));
      /* parse_voltages(response, 14); */

      // Then query the temperature
      uint16_t voltage = 370;
      uint16_t temp = 750;
      // For now, we use a dumb algo
      for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 10; j++) {
          robin->individual_voltages[i][j] =
              voltage; // with factor 0.01 = 3.64V
          robin->individual_temperatures[i][j] = temp; // with factor 0.1 = 75.0
        }
      }

      System_report_to_user(robin);
    }
  }
}

void System_report_to_user(Robin_container_t *robin) {
  individual_temperatures_frame_t Tframe;
  individual_voltages_frame_t Vframe;

  // 28 Muxes (5 Values per Mux)
  // 1 -28
  // 0x01 - 0x1C
  uint16_t first_half[5];
  uint16_t second_half[5];
  for (int mux = 1; mux < 29; mux++) {
    // for every pack/ stacks
    for (int pack = 0; pack < 14; pack++) {
      // take 5 values interval
      //
      // for 10 in a stack
      for (int i = 0; i < 5; i++) {
        first_half[i] = robin->individual_voltages[pack][i];
        second_half[i] = robin->individual_voltages[pack][i + 5];
      }
      // send first 5
      individual_voltages_formatter(&Vframe, first_half, mux);
      CAN_TX_enqueue(0x202, 8, Vframe.bytes);
      // send last 5
      individual_voltages_formatter(&Vframe, second_half, mux);
      CAN_TX_enqueue(0x202, 8, Vframe.bytes);

      // now Temperatures
      // take 5 values interval
      for (int i = 0; i < 5; i++) {
        first_half[i] = robin->individual_temperatures[pack][i];
        second_half[i] = robin->individual_temperatures[pack][i + 5];
      }
      // send first 5
      individual_temperatures_formatter(&Tframe, first_half, mux);
      CAN_TX_enqueue(0x100, 8, Tframe.bytes);
      // send last 5
      individual_temperatures_formatter(&Tframe, second_half, mux);
      CAN_TX_enqueue(0x100, 8, Tframe.bytes);
    }
  }

  free(robin);
}

/*
 *
 * Parsing Data for and from Communication
 *
 */

// CAN TX

void individual_voltages_formatter(individual_voltages_frame_t *frame,
                                   uint16_t voltages[5], uint8_t mux) {
  memset(frame->bytes, 0, sizeof(frame->bytes));

  frame->fields.mux = mux;
  frame->fields.voltage_1 = voltages[0] & 0x3FF;
  frame->fields.voltage_2 = voltages[1] & 0x3FF;
  frame->fields.voltage_3 = voltages[2] & 0x3FF;
  frame->fields.voltage_4 = voltages[3] & 0x3FF;
  frame->fields.voltage_5 = voltages[4] & 0x3FF;

  frame->fields.Reserved = 0;
}

void individual_temperatures_formatter(individual_temperatures_frame_t *frame,
                                       uint16_t temps[5], uint8_t mux) {
  frame->fields.mux = mux;
  frame->fields.temp_1 = temps[0] & 0x3FF;
  frame->fields.temp_2 = temps[1] & 0x3FF;
  frame->fields.temp_3 = temps[2] & 0x3FF;
  frame->fields.temp_4 = temps[3] & 0x3FF;
  frame->fields.temp_5 = temps[4] & 0x3FF;

  frame->fields.Reserved = 0;
}

// SPI RX
void parse_voltages(uint8_t *response, size_t num_modules) {
  /*
   *
   * For every module, Get the 10 x 16 Bits (2 Bytes) values of the cell
   * voltages
   *
   */

  for (int module = 0; module < num_modules; module++) {
    printf("Module: %d", module + 1);
    for (int cell = 0; cell < 10; cell++) {
      uint16_t raw_voltage = (response[module * 34 + cell * 2] << 8 |
                              response[module * 34 + cell * 2 + 1]);
      printf("Value Module %d, cell %d: %f \n", module, cell,
             (raw_voltage * 0.00015));
    }
  }
}
