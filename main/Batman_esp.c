#include "Batman_esp.h"
#include "OTA.h"
#include "Wifi.h"
#include "can_handler.h"
#include "diagnostic_handler.h"
#include "driver/twai.h" // TWAI is Espressif's CAN driver in ESP-IDF
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "include/Adbms_handler.h"
#include "spi_handler.h"
#include <stdio.h>

static const char *TAG = "Setup";

void initialise_setups(void) {
  // Setup

  CAN_Setup();
  SPI_Setup();
  WIFI_Setup();

  // Setup Diagnostic system
  Diag_Setup();
  ADBMS_Setup();
}

void Start_schedule(void) {

  // Start Timer scheduling
  if (xTimerStart(diagnostic_timer, 0) != pdPASS) {
    ESP_LOGE(TAG, "[-] Failed to start diagnostic_timer\n");
  }

  if (xTimerStart(adbms_timer, 0) != pdPASS) {
    ESP_LOGE(TAG, "[-] Failed to start adbms_timer");
  }

  // Schedule a ADBMS query every 500ms (test case) // can be higher frequency
  //
  // Upon created, tasks are placed into ready state
  xTaskCreate(Diagnostic_check, "System_diag", 2048, NULL, 4, NULL);
  xTaskCreate(CAN_sendMessage, "Can_tx", 2048, NULL, 3, NULL);
  xTaskCreate(Robin_query, "ADBMS_query", 2048, NULL, 2, NULL);
}

void app_main(void) {

  initialise_setups();
  ESP_LOGI(TAG, "[+] System Initialised\n");
  Start_schedule();
}
