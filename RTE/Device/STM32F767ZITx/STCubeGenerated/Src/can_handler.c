#include "Driver_CAN.h"
#include "stm32f7xx_hal.h"
#include "config.h"

uint32_t                        rx_obj_idx  = 0xFFFFFFFFU;
uint8_t                         rx_data[8];
ARM_CAN_MSG_INFO                rx_msg_info;
uint32_t                        tx_obj_idx  = 0xFFFFFFFFU;
uint8_t                         tx_data[8];
ARM_CAN_MSG_INFO                tx_msg_info;

// filtering received messages and choose the appropriate actuators 
void RX_msg_filter(ARM_DRIVER_CAN *can_driver) {
    if (can_driver->MessageRead(rx_obj_idx, &rx_msg_info, rx_data, 8U) > 0U ) {

    } 
}


// this will break up the structure and send the frame on the CAN bus network
void TX_msg() {

}
