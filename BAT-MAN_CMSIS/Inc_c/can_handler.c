#include "can_handler.h"
#include "stm32f767xx.h"
#include "core_cm7.h"

void CAN1_INIT() {

    // Enable Peripheral Clock
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;


    // Enable GPIO CLock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configure AF9
    GPIOA->MODER &= ~((3U << (11 * 2)) | (3U << (12 * 2)));  // Clear
    GPIOA->MODER |= (2U << (11 * 2)) | (2U << (12 * 2));  // Alternate Function

    // Set to AF9 (CAN1)
    GPIOA->AFR[1] &= ~((0xF << (4 * 3)) | (0xF << (4 * 4)));  // Clear
    GPIOA->AFR[1] |= (9U << (4 * 3)) | (9U << (4 * 4));       // Set AF9

    // Set Output type (push-pull), no pull-up/pull-down resistors
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT11 | GPIO_OTYPER_OT12);
    GPIOA->PUPDR &= ~((3U << (11 * 2)) | (3U << (12 * 2)));

    // Enter Initialisation (To configure the Baudrate) 
    CAN1->MCR |= CAN_MCR_INRQ;
    while (!(CAN1->MSR & CAN_MCR_INRQ)){;}

    /*
     +
     +  CAN Bit Rate =         APB1 Clock
     +                ----------------------------
     +                  (BRP + 1) x (TS1 + TS2 + 1)
     +
     +  BRP: Baud Rate Prescaler
     +  TS1: Time Segment 1
     +  TS2: Time Segment 2
     +
     +
     */ 

     CAN1->BTR |= (5U << CAN_BTR_BRP_Pos) 
               |  (13U << CAN_BTR_TS1_Pos)
               |  (2U << CAN_BTR_TS2_Pos)
               |  (0U << CAN_BTR_SJW_Pos);  // Sync Jump Width

    // Enter Normal Mode
    CAN1->MCR |= ~CAN_MCR_INRQ;
    // Until Initialisation is Acknowledged
    while (CAN1->MSR & CAN_MSR_INAK){;} 

    // Filters 
    CAN1->FMR |= CAN_FMR_FINIT;

    // ! MUST SET IT TO 0x173
    // Configure Filter 0 to accept all messages 
    CAN1->FA1R &= ~(1U << 0);   // Deactivate Filter 0
    CAN1->FS1R |= (1U << 0);    // Set to Single 32 Filter
    CAN1->FM1R &= ~(1U << 0);   // set to mask mode
    CAN1->FFA1R &= ~(1U << 0);  // Assign Filter 0 

    // Set filter to accept all messages 
    CAN1->sFilterRegister[0].FR1 = 0x00000000;  // Filter ID
    CAN1->sFilterRegister[0].FR2 = 0x00000000;  // Filter Mask 

    CAN1->FA1R |= (1U << 0);    // Activate Filter 0

    // Exit Filter initialisation
    CAN1->FMR |= ~CAN_FMR_FINIT;

    // Enable CAN interrupts
    CAN1->IER |= CAN_IER_FMPIE0 | CAN_IER_TMEIE;
}

void CAN1_TX_IRQHandler() {
    if (CAN1->TSR & CAN_TSR_RQCP0) {
        CAN1->TSR |= CAN_TSR_RQCP0; // Clear Interrupt Flag
    } 
}

void CAN1_RX0_IRQHandler() {
    if (CAN1->RF0R & CAN_RF0R_FMP0) {
        // Handle Message


        // Clear Flags
        CAN1->RF0R |= CAN_RF0R_RFOM0; // release FIFO 0
    }
}


void CAN1_Tx_Data() {
    // Load CAN Messages (Example)
    CAN1->sTxMailBox[0].TIR |= (0x123U << CAN_TI0R_STID_Pos);   // Standard ID = 123
    CAN1->sTxMailBox[0].TDTR |= 2U;   // Data length = 2
    CAN1->sTxMailBox[0].TDLR |= (0x22U << 8) | 0x11U;   // Data bytes = 0x11, 0x22

    // Request Transmission
    CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;

    // Wait for Transmission to finish
    while ((CAN1->TSR & CAN_TSR_RQCP0) == 0); 
}

void CAN1_Rx_Data() {
    // Check if a message is pending in FIFO 0
    if (CAN1->RF0R & CAN_RF0R_FMP0) {
        uint32_t id = CAN1->sFIFOMailBox[0].RIR >> 21;  // Get Standard ID
        uint8_t len = CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC;  // Get Data length
        uint8_t data[8];
        data[0] = CAN1->sFIFOMailBox[0].RDLR & 0xFF;
        data[1] = (CAN1->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
        // Read additional data bytes as needed

        // Release the FIFO
        CAN1->RF0R |= CAN_RF0R_RFOM0;
    }
}


// CAN2
void CAN2_INIT() {

    // Enable Peripheral Clock
    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;


    // Enable GPIO CLock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configure AF9
    GPIOA->MODER &= ~((3U << (11 * 2)) | (3U << (12 * 2)));  // Clear
    GPIOA->MODER |= (2U << (11 * 2)) | (2U << (12 * 2));  // Alternate Function

    // Set to AF9 (CAN1)
    GPIOA->AFR[1] &= ~((0xF << (4 * 3)) | (0xF << (4 * 4)));  // Clear
    GPIOA->AFR[1] |= (9U << (4 * 3)) | (9U << (4 * 4));       // Set AF9

    // Set Output type (push-pull), no pull-up/pull-down resistors
    GPIOA->OTYPER &= ~(GPIO_OTYPER_OT11 | GPIO_OTYPER_OT12);
    GPIOA->PUPDR &= ~((3U << (11 * 2)) | (3U << (12 * 2)));

    // Enter Initialisation (To configure the Baudrate) 
    CAN2->MCR |= CAN_MCR_INRQ;
    while (!(CAN2->MSR & CAN_MCR_INRQ)){;}

    /*
     +
     +  CAN Bit Rate =         APB1 Clock
     +                ----------------------------
     +                  (BRP + 1) x (TS1 + TS2 + 1)
     +
     +  BRP: Baud Rate Prescaler
     +  TS1: Time Segment 1
     +  TS2: Time Segment 2
     +
     +
     */ 

     CAN2->BTR |= (5U << CAN_BTR_BRP_Pos)
               |  (13U << CAN_BTR_TS1_Pos)
               |  (2U << CAN_BTR_TS2_Pos)
               |  (0U << CAN_BTR_SJW_Pos);  // Sync Jump Width

    // Enter Normal Mode
    CAN2->MCR |= ~CAN_MCR_INRQ;
    // Until Initialisation is Acknowledged
    while (CAN2->MSR & CAN_MSR_INAK){;}

    // Filters 
    CAN2->FMR |= CAN_FMR_FINIT;

    // Configure Filter 0 to accept all messages 
    CAN2->FA1R &= ~(1U << 0);   // Deactivate Filter 0
    CAN2->FS1R |= (1U << 0);    // Set to Single 32 Filter
    CAN2->FM1R &= ~(1U << 0);   // set to mask mode
    CAN2->FFA1R &= ~(1U << 0);  // Assign Filter 0

    // Set filter to accept all messages 
    CAN2->sFilterRegister[0].FR1 = 0x00000000;  // Filter ID
    CAN2->sFilterRegister[0].FR2 = 0x00000000;  // Filter Mask

    CAN2->FA1R |= (1U << 0);    // Activate Filter 0

    // Exit Filter initialisation
    CAN2->FMR |= ~CAN_FMR_FINIT;

    // Enable CAN interrupts
    CAN2->IER |= CAN_IER_FMPIE0 | CAN_IER_TMEIE;
}


// MCU -> 
void CAN2_TX_IRQHandler() {
    if (CAN2->TSR & CAN_TSR_RQCP0) {
        CAN2->TSR |= CAN_TSR_RQCP0; // Clear Interrupt Flag

    } 
}

void CAN2_RX0_IRQHandler() {
    if (CAN2->RF0R & CAN_RF0R_FMP0) {
        // Handle Message


        // Clear Flags
        CAN2->RF0R |= CAN_RF0R_RFOM0; // release FIFO 0
    }
}


void CAN2_Tx_Data() {
    // Load CAN Messages (Example)
    CAN2->sTxMailBox[0].TIR |= (0x123U << CAN_TI0R_STID_Pos);   // Standard ID = 123
    CAN2->sTxMailBox[0].TDTR |= 2U;   // Data length = 2
    CAN2->sTxMailBox[0].TDLR |= (0x22U << 8) | 0x11U;   // Data bytes = 0x11, 0x22

    // Request Transmission
    CAN2->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ;

    // Wait for Transmission to finish
    while ((CAN2->TSR & CAN_TSR_RQCP0) == 0);
}

void CAN2_Rx_Data() {
    // Check if a message is pending in FIFO 0
    if (CAN2->RF0R & CAN_RF0R_FMP0) {
        uint32_t id = CAN2->sFIFOMailBox[0].RIR >> 21;  // Get Standard ID
        uint8_t len = CAN2->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC;  // Get Data length
        uint8_t data[8];
        data[0] = CAN2->sFIFOMailBox[0].RDLR & 0xFF;
        data[1] = (CAN2->sFIFOMailBox[0].RDLR >> 8) & 0xFF;
        // Read additional data bytes as needed

        // Release the FIFO
        CAN2->RF0R |= CAN_RF0R_RFOM0;
    }
}