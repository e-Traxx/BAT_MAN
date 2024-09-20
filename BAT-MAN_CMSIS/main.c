#include "uart_handler.h"
#include "usb_handler.h"
#include "stm32f767xx.h"
#include "can_handler.h"


int main(void) {
    

    // initialise Peripherals
    UART_INIT();
    USB_INIT();
    CAN1_INIT();

    while (1) {
        if (USART3->ISR & USART_ISR_RXNE) {
            U_debug("Received");
        }
    }
}

