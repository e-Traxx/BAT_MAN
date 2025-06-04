#ifndef BATMAN_ESP_H
#define BATMAN_ESP_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include <stdint.h>

// Shows which section is working and not working
typedef struct {
  bool CAN_Error;
  bool SPI_Error;
  bool WIFI_Error;
  bool ADBMS_Error;
  bool ALBERT_Error;
  bool DIAG_Error;
  bool RTOS_Error;
  bool Charging;
} System_health_Flags;

extern System_health_Flags flags;
extern bool SystemInitialised;

#endif // BATMAN_ESP_H
