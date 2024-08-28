#ifndef __CONFIG_H
#define __CONFIG_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "Driver_CAN.h"
#include "Driver_Common.h"
#include "Driver_USART.h"
#include "stm32f7xx_hal.h"
#include <string.h>
#include "cmsis_os2.h"
#include <stdint.h>


/********** UART **********/
void myUART_Thread(void const *argument);
void myUSART_callback(uint32_t event) {}
// CMSIS RTOS Thread - UART 
void myUART_Thread(const void* args) {}


/********** CAN **********/
/*
 * CAN is the primary mode of communication to the ECU. 
 */
#define CAN_CONTROLLER 1 //CAN1
#define _CAN_Driver_(n) Driver_CAN##n
#define CAN_Driver_(n)  _CAN_Driver_(n)
#define ptrCAN (&CAN_Driver_(CAN_CONTROLLER))

void CAN_SignalUnitEvent (uint32_t event) {}

void CAN_SignalObjectEvent (uint32_t obj_idx, uint32_t event) {}

void CAN_INIT() {}
 

/********** ETH **********/


#ifdef __cplusplus
}
#endif

#endif