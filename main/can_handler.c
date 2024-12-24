#include "can_handler.h"
#include "Batman_esp.h"
#include "driver/twai.h"
#include "esp_err.h"
#include "esp_intr_alloc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "hal/twai_types.h"
#include "portmacro.h"
#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#define TX_NUM GPIO_NUM_21
#define RX_NUM GPIO_NUM_22

// handles the Can Rtos Scheduling
QueueHandle_t can_queue;
// extern TaskHandle_t Can_handle = NULL;
SemaphoreHandle_t can_semaphore_start_transmit;

static const twai_timing_config_t t_config = TWAI_TIMING_CONFIG_1MBITS();
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
static const twai_general_config_t g_config = {.mode = TWAI_MODE_NORMAL,
                                               .tx_io = TX_NUM,
                                               .rx_io = RX_NUM,
                                               .clkout_io = TWAI_IO_UNUSED,
                                               .bus_off_io = TWAI_IO_UNUSED,
                                               .tx_queue_len = 5,
                                               .rx_queue_len = 5,
                                               .alerts_enabled = TWAI_ALERT_ALL,
                                               .clkout_divider = 0,
                                               .intr_flags =
                                                   ESP_INTR_FLAG_LEVEL2};

void CAN_Setup(void) {
  // Semaphore setup
  can_semaphore_start_transmit = xSemaphoreCreateMutex();
  if (can_semaphore_start_transmit == NULL) {
    printf("[-] Failed to create CAN Semaphore\n");
  }

  // CAN Queue
  can_queue = xQueueCreate(CAN_QUEUE_SIZE, sizeof(can_message_t *));
  if (can_queue == NULL) {
    printf("[-] Failed to create CAN queue\n");
  }

  // Install and start TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    if (twai_start() == ESP_OK) {
      printf("[+] TWAI Driver installed and started\n");
    } else {
      printf("[-] Failed to Start TWAI\n");
      vTaskDelete(NULL);
    }
  } else {
    printf("[-] Failed to Install TWAI\n");
  }
};

void CAN_sendMessage(void *arguments) {
  can_message_t *msg_arg; // Buffer (pointer to MSG)

  while (1) {

    // take data present on the Queue
    if (xQueueReceive(can_queue, &msg_arg, portMAX_DELAY) == pdPASS) {
      printf("[*] Received MSG with ID %" PRIu32 " and DLC %" PRIu8 "\n",
             msg_arg->identifier, msg_arg->DLC);

      // Notice to start operation (take mutex)
      printf("[*] Starting Can Transmission\n");

      twai_message_t msg = {
          .identifier = msg_arg->identifier,
          .data_length_code = msg_arg->DLC,
          .rtr = 0,
      };
      // pack data into msg
      memcpy(msg.data, msg_arg->data, msg_arg->DLC);

      // send Can Message
      esp_err_t res = twai_transmit(&msg, pdMS_TO_TICKS(200));
      if (res == ESP_OK) {
        printf("[+] CAN Message Sent\n"); // remove later, just for testing
      } else {
        printf("[-] Failed to send Message\n");
      }
    } else {
      printf("[-] Did not receive Msg from Queue\n");
    }
    // free the allocated memory after use
    free(msg_arg);
  }
}

void CAN_TX_enqueue(uint32_t identifier, uint8_t DLC, uint8_t data[8]) {
  // format msg
  can_message_t *msg = malloc(sizeof(can_message_t));
  if (msg == NULL) {
    printf("[-] Failed to allocate memory for CAN Message\n");
  }

  // format msg
  msg->identifier = identifier;
  msg->DLC = DLC;
  memcpy(&msg->data, data, msg->DLC);

  // enqueue message
  if (xQueueSend(can_queue, &msg, portMAX_DELAY) != pdPASS) {
    printf("[-] Failed to enqueue CAN Message\n");
    free(msg);
  }
}

void Can_TX_callback() {}
