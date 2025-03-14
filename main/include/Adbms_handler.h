#ifndef ADBMSH_H
#define ADBMSH_H

#include "freertos/idf_additions.h"
#include "stdint.h"
#include <stdint.h>

#define NUM_STACKS 14
#define CELLS_PER_STACK_ACTIVE 10
#define CELLS_PER_STACK 18
#define RESPONSE_SIZE 13 // BYTES
#define TOTAL_CELLS (NUM_STACKS * CELLS_PER_STACK)

void ADBMS_Setup(void);
void Adbms_query_callback(TimerHandle_t timer);
void Robin_query(void *args);
uint8_t calculate_PEC(const uint8_t *data, size_t len);

typedef struct {
  uint16_t individual_voltages[14][10];
  uint16_t individual_temperatures[14][10];
} Robin_container_t;

// CAN Message Template
typedef struct {
  uint32_t mux : 8;
  uint32_t voltage_1 : 10;
  uint32_t voltage_2 : 10;
  uint32_t voltage_3 : 10;
  uint32_t voltage_4 : 10;
  uint32_t voltage_5 : 10;

  uint32_t Reserved : 6;

} __attribute__((packed)) individual_voltages_message_t;

typedef struct {
  uint32_t mux : 8;
  uint32_t temp_1 : 10;
  uint32_t temp_2 : 10;
  uint32_t temp_3 : 10;
  uint32_t temp_4 : 10;
  uint32_t temp_5 : 10;

  uint32_t Reserved : 6;

} __attribute__((packed)) individual_temperatures_message_t;

// The physical frame in bytes
typedef union {
  individual_voltages_message_t fields;
  uint8_t bytes[8];
} individual_voltages_frame_t;

typedef union {
  individual_temperatures_message_t fields;
  uint8_t bytes[8];
} individual_temperatures_frame_t;

// SPI Transmission
typedef union {
  // Each stack has 18 Cells
  // and we have 14 Stacks in total which amount to NUM_STACKS * CELLS_PER_STACK
  // 16-bit values to store
  uint8_t raw[((NUM_STACKS * CELLS_PER_STACK) + 1) * 2];
  uint16_t values[(NUM_STACKS * CELLS_PER_STACK) + 1];
} SPI_responses_t;

// Command description Structure
typedef struct {
  // 5-bit Address
  uint8_t address;
  // Addressed command or broadcasted command
  bool broadcast;
  // 11 Bit command
  uint16_t command;
} spi_user_command_t;

// Serial ID
extern uint8_t ADBMS_ID[(6 * NUM_STACKS) + 2];

// handlers
extern Robin_container_t *robin;
extern SemaphoreHandle_t adbms_semaphore;
extern TimerHandle_t adbms_timer;

#endif
