#ifndef DIAGH_H
#define DIAGH_H

#include "Batman_esp.h"
#include "freertos/idf_additions.h"
#include <stdint.h>

void Diag_Setup(void);
void Diagnostic_check(void *argumensts);

extern SemaphoreHandle_t diagnostic_semaphore;
extern TimerHandle_t diagnostic_timer;


// TODO: REMOVE SOH AND FIX BIT ALLOCATIONS
typedef struct {
  uint32_t Overall_voltage : 13; // 0-12 Bits
  uint32_t Highest_temp : 14;    // 13-26 Bits
  uint32_t Curr_value : 12;      // 27-38 Bits
  uint32_t SOC : 8;              // 39-46 Bits
  uint32_t SOH : 8;              // 47-54 Bits

  // Error Flags
  uint32_t lost_comm : 1;           // Bit 55
  uint32_t Voltage_sense_error : 1; // Bit 56
  uint32_t Overall_overvoltage : 1; // Bit 57
  uint32_t Balancing_on : 1;        // Bit 58
  uint32_t temp_sensor_loss : 1;    // Bit 59
  uint32_t pack_undervoltage : 1;   // Bit 60
  uint32_t Curr_sensor_loss : 1;    // Bit 61
  uint32_t high_temp : 1;           // Bit 62

  uint32_t Reserved : 1;
} Diag_message_t;

// Union to access both as Struct and byte array
typedef union {
  Diag_message_t fields;
  uint8_t bytes[8];
} diag_frame_t;

// Diagnostic Flags and values
typedef struct {
  uint16_t overall_voltage;
  uint16_t current;
  uint16_t temp;
  uint8_t soc;
  uint8_t soh;

  uint8_t flags[8];
} Diagnostic_Container_t;

#endif
