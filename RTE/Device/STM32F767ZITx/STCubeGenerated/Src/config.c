#include "Driver_CAN.h"
#include "Driver_Common.h"
#include "Driver_USART.h"
#include "stm32f7xx_hal.h"
#include <string.h>
#include "cmsis_os2.h"
#include <stdint.h>
#include "can_handler.h"


/********** USART **********/
/*
 * USART is an important part for debugging,
 * Using minicom or any other serial monitor, we can communicate and output all error that are present.
 */
void myUART_Thread(void const *argument);
osThreadId_t tid_myUART_Thread;

// USART Driver
extern ARM_DRIVER_USART ARM_DRIVER_USART3;

void myUSART_callback(uint32_t event) {
    uint32_t mask;

     mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
            ARM_USART_EVENT_TRANSFER_COMPLETE |
            ARM_USART_EVENT_SEND_COMPLETE     |
            ARM_USART_EVENT_TX_COMPLETE       ;

    if (event & mask) {
        // Success: Wakeup thread
        // ! CMSIS RTOS 2
        osThreadFlagsSet(tid_myUART_Thread, 0x01);
    }   

    // Error to be implemented
    if (event & ARM_USART_EVENT_RX_TIMEOUT) {
    }

    if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
    }
}

// CMSIS RTOS Thread - UART 
void myUART_Thread(const void* args) {
    static ARM_DRIVER_USART * USARTdrv = &ARM_DRIVER_USART3;
    ARM_DRIVER_VERSION version;
    ARM_USART_CAPABILITIES drv_capabilitied;
    char cmd;

    #ifdef DEBUG 
        version = USART->GetVersion();
        if (version.api < 0x20) {
            // requires minimum version 2.0
            return;
        } 
        drv_capabilities = USARTdrv->GetCapabilities();
        if (drv_capabilities.event_tx_complete == 0) {
            return;
        }

    #endif

    // Initialise USART Driver 
    USARTdrv->Initialize(myUSART_callback);
    // Power up peripheral 
    USARTdrv->PowerControl(ARM_POWER_FULL);
    // Configuration
    USARTdrv->Control(  ARM_USART_MODE_ASYNCHRONOUS |
                        ARM_USART_DATA_BITS_8       |
                        ARM_USART_PARITY_NONE       |
                        ARM_USART_STOP_BITS_1       |
                        ARM_USART_FLOW_CONTROL_NONE ,
                        4800);

    // Enable TX and RX
    USARTdrv->Control(ARM_USART_CONTROL_TX, 1);
    USARTdrv->Control(ARM_USART_CONTROL_RX, 1);

    // * EXAMPLE CODE
    // USARTdrv->Send("\nPress Enter to receive a message", 34);
    // osSignalWait(0x01, osWaitForever);
     
    // while (1)
    // {
    //     USARTdrv->Receive(&cmd, 1);          /* Get byte from UART */
    //     osSignalWait(0x01, osWaitForever);
    //     if (cmd == 13)                       /* CR, send greeting  */
    //     {
    //       USARTdrv->Send("\nHello World!", 12);
    //       osSignalWait(0x01, osWaitForever);
    //     }
 
    // }
}


/********** CAN **********/
/*
 * CAN is the primary mode of communication to the ECU. 
 */
 #define CAN_CONTROLLER 1 //CAN1

#define _CAN_Driver_(n) Driver_CAN##n
#define CAN_Driver_(n)  _CAN_Driver_(n)
extern ARM_DRIVER_CAN CAN_Driver_(CAN_CONTROLLER);

#define ptrCAN (&CAN_Driver_(CAN_CONTROLLER))

uint32_t                        rx_obj_idx  = 0xFFFFFFFFU;
uint8_t                         rx_data[8];
ARM_CAN_MSG_INFO                rx_msg_info;
uint32_t                        tx_obj_idx  = 0xFFFFFFFFU;
uint8_t                         tx_data[8];
ARM_CAN_MSG_INFO                tx_msg_info;

void CAN_SignalUnitEvent (uint32_t event) {}

void CAN_SignalObjectEvent (uint32_t obj_idx, uint32_t event) {
 
  if (obj_idx == rx_obj_idx) {                  // If receive object event
    if (event == ARM_CAN_EVENT_RECEIVE) {       // If message was received successfully
        RX_msg_filter(ptrCAN);
    //   if (ptrCAN->MessageRead(rx_obj_idx, &rx_msg_info, rx_data, 8U) > 0U) {
      //   // Read received message
      //   // process received message ...

      
      // }
    }
  }
  if (obj_idx == tx_obj_idx) {                  // If transmit object event
    if (event == ARM_CAN_EVENT_SEND_COMPLETE) { // If message was sent successfully
      // acknowledge sent message ...
    }
  }
}

void CAN_INIT() {

}
 

/********** ETH **********/
