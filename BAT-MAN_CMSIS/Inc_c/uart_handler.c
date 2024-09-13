#include "uart_handler.h"
#include "stm32f767xx.h" 
#include <string.h>

/*
 * Debugging and Control Module
 */


// AF = Alternate Function template
// AF7 refers to PD8(Tx), PD9(RX), PD10(CK), PD11(CTS), PD12(RTS)  
#define AF07 0x07


void USART_Tx_Data(unsigned char x);

// Reference material 
//?https://www.st.com/content/ccc/resource/technical/document/datasheet/group3/c5/37/9c/1d/a6/09/4e/1a/DM00273119/files/DM00273119.pdf/jcr:content/translations/en.DM00273119.pdf
// ?https://blog.embeddedexpert.io/?p=640
void UART_INIT() {

    // Enable Clock access to PD8 
    RCC->AHB1ENR|= RCC_AHB1ENR_GPIODEN;

    // Set to Alternate Function Mode
    // https://blog.embeddedexpert.io/?p=562
    GPIOD->MODER|=GPIO_MODER_MODER8_1;

    // refer to Alternate Function Mapping 
    GPIOD->AFR[1] |= (AF07<<0);

    // Enable Clock access to USART3
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    // Baudrate
    USART3->BRR |= 0x008B; // 115200 16Mhz 

    // Enable the TX port (Bit 3)
    USART3->CR1 |= USART_CR1_TE;

    // Enable Module 
    USART3->CR1 |= USART_CR1_UE;

} 

void USART_Tx_Data(unsigned char x) {
    // 8 bits to store the to be transmitted value
    // Send data to transfer buffer 
    USART3->TDR = (x);

    // wait until transfer is complete
    /*     * Until the Bit 6 is a 1, showing a completed transfer
     */
    
    while (!((USART3->ISR) & USART_ISR_TC)) {
        ; // wait until end of transfer
    }
}


// TODO: Implement Error Handling
int U_debug(char Tx[]){
    int len = strlen(Tx);
    
    // Send Every 8 bit char individually
    for (int i=0; i < len; i++) {
        USART_Tx_Data(Tx[i]);
    }
    
    // Formatting
    USART_Tx_Data('\r');
    USART_Tx_Data('\n');
    

    return 1;
}