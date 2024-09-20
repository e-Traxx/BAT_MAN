#ifndef __CANH_H
#define __CANH_H

#ifdef __cplusplus
 extern "C" {
#endif

// Includes 
void CAN1_INIT(void);
void CAN1_TX_IRQHandler(void);
void CAN1_Tx_Data(void);

void CAN1_RX_IRQHandler(void);
void CAN1_Rx_Data(void);


#ifdef __cplusplus
}
#endif

#endif