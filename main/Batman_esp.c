#include "Batman_esp.h"
#include "can_handler.h"
#include "diagnostic_handler.h"
#include "driver/twai.h" // TWAI is Espressif's CAN driver in ESP-IDF
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include <stdio.h>

void initialise_setups(void) {
  // Setup

  CAN_Setup();

  // Setup Diagnostic system
  Diag_Setup();
}

void Start_schedule(void) {

  // Start Timer scheduling
  if (xTimerStart(diagnostic_timer, 0) != pdPASS) {
    printf("[-] Failed to start diagnostic_timer\n");
  }

  // Schedule a ADBMS query every 500ms (test case) // can be higher frequency
  //
  // Upon created, tasks are placed into ready state
  // xTaskCreate(ADBMS_query, "Batman_signal", 2048, NULL, 2, NULL);
  xTaskCreate(Diagnostic_check, "System_diag", 2048, NULL, 4, NULL);
  xTaskCreate(CAN_sendMessage, "Can_tx", 2048, NULL, 3, NULL);
}

void app_main(void) {

  initialise_setups();
  printf("[+] System Initialised\n");
  Start_schedule();
}
