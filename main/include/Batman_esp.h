#ifndef BATMAN_ESP_H
#define BATMAN_ESP_H

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include <stdint.h>

// Shows which section is working and not working
typedef struct {
  unsigned CAN_Error;
  unsigned SPI_Error;
  unsigned WIFI_Error;
  unsigned ADBMS_Error;
  unsigned ALBERT_Error;
  unsigned DIAG_Error;
  unsigned RTOS_Error;
} System_health_Flags;

extern System_health_Flags flags;
extern bool SystemInitialised;

#endif // BATMAN_ESP_H
