#ifndef ADBMSH_H
#define ADBMSH_H

#include "freertos/idf_additions.h"
#include "stdint.h"

void ADBMS_Setup(void);
void Adbms_query_callback(TimerHandle_t timer);
void Robin_query(void *args);

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

extern Robin_container_t *robin;
extern SemaphoreHandle_t adbms_semaphore;
extern TimerHandle_t adbms_timer;

#endif
