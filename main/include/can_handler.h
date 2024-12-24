#ifndef CANH_H
#define CANH_H

#include "freertos/idf_additions.h"
#include <stdint.h>

#define CAN_QUEUE_SIZE 20
#define CAN_MAX_DATA_LENGTH 8

void CAN_Setup(void);
void CAN_sendMessage(void *arguments);
void CAN_TX_enqueue(uint32_t identifier, uint8_t DLC, uint8_t data[8]);

// handles the Can Rtos Scheduling
extern QueueHandle_t can_queue;
// extern TaskHandle_t Can_handle = NULL;
extern SemaphoreHandle_t can_semaphore_start_transmit;

// Standard structure of CAN messages
typedef struct {
  uint32_t identifier;
  uint8_t DLC;
  uint8_t data[CAN_MAX_DATA_LENGTH];
} can_message_t;

#endif // MY_FUNCTIONS_H
