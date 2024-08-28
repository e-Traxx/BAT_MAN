#ifndef __CAN_HANDLER_H
#define __CAN_HANDLER_H

#ifdef __cplusplus
 extern "C" {
#endif


#include "Driver_CAN.h"
#include "stm32f7xx_hal.h"



// filtering received messages and choose the appropriate actuators 
void RX_msg_filter();


#ifdef __cplusplus
}
#endif

#endif