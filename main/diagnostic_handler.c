#include "diagnostic_handler.h"
#include "Batman_esp.h"
#include "can_handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "portmacro.h"
#include "spi_handler.h"
#include <stdint.h>

SemaphoreHandle_t diagnostic_semaphore;
TimerHandle_t diagnostic_timer;

void diagnostic_timer_callback(TimerHandle_t DiagTimer);
void Diagnostic_check(void *arguments);
void Diagnostic_packet_formatter(diag_frame_t *frame, uint16_t voltage,
                                 uint16_t current, uint16_t temp, uint8_t soc,
                                 uint8_t soh, uint8_t flags[8]);

void Diag_Setup(void) {
  // Semaphore Setup
  diagnostic_semaphore = xSemaphoreCreateBinary();
  if (diagnostic_semaphore == NULL) {
    printf("[-] Failed to create semaphore.\n");
    return;
  }

  // Create Timer
  diagnostic_timer = xTimerCreate("Diagnostic_timer", pdMS_TO_TICKS(1000),
                                  pdTRUE, (void *)0, diagnostic_timer_callback);
  if (diagnostic_timer == NULL) {
    printf("[-] Failed to create timer.\n");
    return;
  }
}

// Gives the Semaphore every 200ms
void diagnostic_timer_callback(TimerHandle_t DiagTimer) {
  BaseType_t HighPriorityTaskWoken = pdFALSE;

  // Give Semaphore to Diagnostic_task
  if (xSemaphoreGiveFromISR(diagnostic_semaphore, &HighPriorityTaskWoken) !=
      pdPASS) {
    printf("[-] Failed to give Semaphore to Diagnostic_check\n");
  }
}

void Diagnostic_check(void *arguments) {
  while (1) {
    if (xSemaphoreTake(diagnostic_semaphore, portMAX_DELAY) == pdTRUE) {
      // Perform the diagnostic test
      printf("[*] Running diagnostic test...\n");

      // Insert diagnostic operations here
      // e.g., reading sensors, checking system health, etc.
      //
      diag_frame_t frame;
      uint16_t overall_voltage = 5600;
      uint16_t current = 40;
      uint16_t temp = 400;
      uint8_t soc = 80;
      uint8_t soh = 80;

      uint8_t flags[8] = {0, 0, 0, 0, 0, 0, 0, 0};

      // Check Sensors Loss

      // Check limits

      // pack values

      // Simulate diagnostic duration
      vTaskDelay(pdMS_TO_TICKS(200)); // 100 ms
      Diagnostic_packet_formatter(&frame, overall_voltage, current, temp, soc,
                                  soh, flags);

      // enqueue data to CAN_TX send function
      CAN_TX_enqueue(0x199, 8, frame.bytes);

      // for now
      spi_read_data();

      printf("[+] Diagnostic test completed.\n");
    }
  }
}

void Diagnostic_packet_formatter(diag_frame_t *frame, uint16_t voltage,
                                 uint16_t current, uint16_t temp, uint8_t soc,
                                 uint8_t soh, uint8_t flags[8]) {

  frame->fields.Overall_voltage = voltage & 0x1FFF;
  frame->fields.Curr_value = current & 0xFFF;
  frame->fields.Highest_temp = temp & 0x3FFF;
  frame->fields.SOC = soc;
  frame->fields.SOH = soh;

  frame->fields.lost_comm = (flags[0] & 0x1);
  frame->fields.Voltage_sense_error = (flags[1] & 0x1);
  frame->fields.Overall_overvoltage = (flags[2] & 0x1);
  frame->fields.Balancing_on = (flags[3] & 0x1);
  frame->fields.temp_sensor_loss = (flags[4] & 0x1);
  frame->fields.pack_undervoltage = (flags[5] & 0x1);
  frame->fields.Curr_sensor_loss = (flags[6] & 0x1);
  frame->fields.high_temp = (flags[7] & 0x1);

  frame->fields.Reserved = 0;
}
