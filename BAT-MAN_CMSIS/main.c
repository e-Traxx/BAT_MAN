#include "uart_handler.h"
#include "usb_handler.h"
#include "stm32f767xx.h"
#include "can_handler.h"
#include "spi_handler.h"
#include "cmsis_os2.h"
#include "adbms_handler.h"

int main(void) {
    

    // initialise Peripherals
    UART_INIT();
    USB_INIT();
    CAN1_INIT();
    SPI_INIT();

    // Initialise CMSIS RTOS
    osKernelInitialize();
    
    // Message Queues for Data transfer between Threads 
    osMessageQueueId_t data_queue_id = osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, NULL);
    osMessageQueueId_t can_queue_id = osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, NULL);

    // Create Threads
    osThreadNew(spi, void *argument, const osThreadAttr_t *attr)

    while (1) {
        if (USART3->ISR & USART_ISR_RXNE) {
            U_debug("Received");
        }
    }
}

