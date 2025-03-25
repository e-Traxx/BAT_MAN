#ifndef ALBERTH_H
#define ALBERTH_H


#include "Batman_esp.h"
#include "freertos/idf_additions.h"
#include <stdint.h>

void Albert_Setup(void);
void Albert_timer_callback(TimerHandle_t timer);
void Albert_Query(void *args);

extern SemaphoreHandle_t albert_semaphore;
extern TimerHandle_t albert_timer;

#endif