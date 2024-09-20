#include "uart_handler.h"
#include "stm32f767xx.h" 
#include <stdint.h>
#include <string.h>

/*
 * Debugging and Control Module
 */


/*
 ! Framing Error is being Generated, dont know why
 TODO: Implement UTF8 support 
 */


// AF = Alternate Function template
// AF7 refers to PD8(Tx), PD9(RX), PD10(CK), PD11(CTS), PD12(RTS)  
#define AF07 0x07
#define BUFF_SIZE 128

// Max of 128 Characters
uint8_t rx_buffer[BUFF_SIZE];
volatile uint8_t rx_read_pos = 0;
volatile uint8_t rx_write_pos = 0;


// Reference material 
//?https://www.st.com/content/ccc/resource/technical/document/datasheet/group3/c5/37/9c/1d/a6/09/4e/1a/DM00273119/files/DM00273119.pdf/jcr:content/translations/en.DM00273119.pdf
// ?https://blog.embeddedexpert.io/?p=640
void UART_INIT() {
    // THE BLINKING PB7
    // Enable the clock for GPIO port B (AHB1 peripheral clock enable register)
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    // Set PB7 as output (MODER7 = 01)
    GPIOB->MODER &= ~(3UL << (2 * 7)); // Clear MODER7 bits
    GPIOB->MODER |= (1UL << (2 * 7));  // Set MODER7 to 01
    // Set the output type as push-pull (OTYPER7 = 0)
    GPIOB->OTYPER &= ~(1UL << 7);
    // Set the output speed to low (OSPEEDR7 = 00)
    GPIOB->OSPEEDR &= ~(3UL << (2 * 7));
    // Set no pull-up/pull-down resistors (PUPDR7 = 00)
    GPIOB->PUPDR &= ~(3UL << (2 * 7));

    // Shows the USART is activated
    GPIOB->ODR |= 0x01;

    // USART
    // Enable Clock access to PD8 
    RCC->AHB1ENR|= RCC_AHB1ENR_GPIODEN;

    // Set to Alternate Function Mode
    // https://blog.embeddedexpert.io/?p=562
    GPIOD->MODER|=GPIO_MODER_MODER8_1;
    // refer to Alternate Function Mapping 
    GPIOD->AFR[1] |= (AF07<<0);

    // Enable Clock access to USART3
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
    GPIOD->PUPDR |= GPIO_PUPDR_PUPDR9_0; // enable pull up resistor on PD9 (Rx Pin) 

    // RX/TX parameters
    USART3->CR1 |= USART_CR1_RXNEIE;
    // STOP [1:0]
    USART3->CR2 |= USART_CR2_STOP_1;
    // // Parity
    // USART3->CR1 |= USART_CR1_PCE;
    // default is Even Parity

    // Enable the TX port (Bit 3)
    USART3->CR1 |= USART_CR1_TE | USART_CR1_RE;
    
    // Enable Error Interrupt
    USART3->CR3 |= USART_CR3_EIE;

    // Baudrate
    USART3->BRR |= 0x008B; // 115200 16Mhz 
    // Enable Module 
    USART3->CR1 |= USART_CR1_UE;

    // Reseting Interrupt states
    USART3->ICR |= USART_ICR_FECF;
    USART3->ICR |= USART_ICR_ORECF;
    USART3->ICR |= USART_ICR_PECF;
    USART3->ICR |= USART_ICR_NCF;


    // Enable Interrupt detection for USART3
    NVIC_SetPriority(USART3_IRQn, 0);
    NVIC_EnableIRQ(USART3_IRQn);
} 

void USART_Tx_Data(unsigned char x) {
    // 8 bits to store the to be transmitted value
    // Send data to transfer buffer 
    USART3->TDR = (x);

    // wait until transfer is complete
    /*     * Until the Bit 6 is a 1, showing a completed transfer
    */
    while (!((USART3->ISR) & USART_ISR_TC)) {;}
}


// TODO: Implement Error Handling
int U_debug(char Tx[]){
    int len = strlen(Tx);

     // Toggle PB7
    // Send Every 8 bit char individually
    for (int i=0; i < len; i++) {
        USART_Tx_Data(Tx[i]);
    }
    
    // Formatting
    USART_Tx_Data('\r');
    USART_Tx_Data('\n');

    return 1;
}


/* RECEIVE DATA*/
/* When a character is received, 
 *  RXNE is set to show that the Byte is received (from interrupt register)
 *  Interrupt is generated
 !  Clearing of the RXNE bit by reading it to avoid Overrun error.
 * The Clearing is done by writing one to the RXFRQ bit of the USART_RQR register. 
*/

// Gets called When Interrupts are generated
void USART3_IRQHandler(void) {
    GPIOB->ODR ^= GPIO_ODR_OD7;

    U_debug("IRQ");

    // Error detection 
    if (((USART3->ISR) & USART_ISR_ORE)) {
        U_debug("[-] UART_Rx Overrun Error");
        USART3->ICR |= USART_ICR_ORECF;
        GPIOB->ODR ^= GPIO_ODR_OD7;
        return;
    }
    if (((USART3->ISR) & USART_ISR_FE)) {
        U_debug("[-] UART_Rx Framing Error");
        USART3->ICR |= USART_ICR_FECF;
        GPIOB->ODR ^= GPIO_ODR_OD7;
        return;
    }
    if (((USART3->ISR) & USART_ISR_NE)) {
        U_debug("[-] UART_Rx Noise Error");
        USART3->ICR |= USART_ICR_NCF;
        GPIOB->ODR ^= GPIO_ODR_OD7;
        return;
    }
    if (((USART3->ISR) & USART_ISR_PE)) {
        U_debug("[-] UART_Rx Parity Error");
        USART3->ICR |= USART_ICR_PECF;
        GPIOB->ODR ^= GPIO_ODR_OD7;
        return;
    }



    // Check status of RXNE
    // if the bit 5 is 1, then run 

    // Explanation of how the syntax work: if the register byte value AND (bit operation) to 
    // the constant USART_ISR_RXNE (which stores a 1 at bit 5), results into 1, then the RX is not empty state can be proved. 
    
    if (((USART3->ISR) & USART_ISR_RXNE)) {
        U_debug("RxNe");
   
        USART3->TDR = USART3->RDR;
        // Read the Byte and store in Buffer
        // rx_buffer[rx_write_pos] = USART3->RDR;
        // rx_write_pos = (rx_write_pos + 1) % BUFF_SIZE;
    }

    GPIOB->ODR ^= GPIO_ODR_OD7;
}

uint8_t USART_receive(){
    uint8_t data = 0;

    if (rx_read_pos != rx_write_pos) {
        data = rx_buffer[rx_read_pos];
        rx_read_pos = (rx_read_pos + 1) % BUFF_SIZE;
    } 
    return data; 
}