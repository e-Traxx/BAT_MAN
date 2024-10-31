#include "stm32f767xx.h"
#include "core_cm7.h"  


void SPI_INIT() {
       
    // Enable SPI1 and GPIOA clocks
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;  // Enable SPI1 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // Enable GPIOA clock

    // Configure PA5 (SCK), PA6 (MISO), and PA7 (MOSI) as alternate functions
    GPIOA->MODER &= ~(GPIO_MODER_MODER5 | GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
    GPIOA->MODER |= (GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1); // Alternate function

    // Select AF5 (SPI1) for PA5, PA6, PA7
    GPIOA->AFR[0] |= (5 << (4 * 5));  // PA5 = SCK (AF5)
    GPIOA->AFR[0] |= (5 << (4 * 6));  // PA6 = MISO (AF5)
    GPIOA->AFR[0] |= (5 << (4 * 7));  // PA7 = MOSI (AF5)  // PA7 = MOSI

    // Configure the SPI1 peripheral
    SPI1->CR1 = 0;  // Clear control register 1
    SPI1->CR1 |= SPI_CR1_MSTR;           // Set as Master
    SPI1->CR1 |= SPI_CR1_BR_0;           // Set baud rate (div by 4)
    SPI1->CR1 |= SPI_CR1_CPOL;           // Clock polarity
    SPI1->CR1 |= SPI_CR1_CPHA;           // Clock phase
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;  // Software slave management enabled
    SPI1->CR1 |= SPI_CR1_SPE;            // Enable SPI

    // Enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;

    // Enable the RXNE (Receive buffer not empty) interrupt
    SPI1->CR2 |= SPI_CR2_RXNEIE;
    // Enable the TXE (Transmit buffer empty) interrupt
    SPI1->CR2 |= SPI_CR2_TXEIE;
    // Enable Error interrupt (optional)
    SPI1->CR2 |= SPI_CR2_ERRIE;

    // Set priority and enable SPI1 interrupt in NVIC
    NVIC_SetPriority(SPI1_IRQn, 1);  // Set priority level
    NVIC_EnableIRQ(SPI1_IRQn);   
    
} 

void SPI1_IRQHandler(void) {
    // Check if the RXNE flag is set (data received)
    if (SPI1->SR & SPI_SR_RXNE) {
        uint8_t received_data = SPI1->DR;  // Read the data
        // Handle received data here (e.g., store in buffer)
    }

    // Check if the TXE flag is set (ready to transmit)
    if (SPI1->SR & SPI_SR_TXE) {
        // Transmit next data if necessary
        // SPI1->DR = <next_data_to_transmit>;
    }

    // Check if there is an overrun error (optional)
    if (SPI1->SR & SPI_SR_OVR) {
        // Clear the OVR flag by reading DR and SR
        uint8_t dummy = SPI1->DR;  // Clear OVR
        (void)SPI1->SR;            // Clear OVR
        // Handle error (if needed)
    }
}

void SPI1_Transmit(uint8_t data) {
    // Wait until TXE (Transmit buffer empty) flag is set
    while (!(SPI1->SR & SPI_SR_TXE));

    // Write data to the SPI data register
    SPI1->DR = data;

    // Wait until the transmission is complete (TXE and BSY flags)
    while (SPI1->SR & SPI_SR_BSY);
}


uint8_t SPI1_Receive(void) {
    // Wait until RXNE (Receive buffer not empty) flag is set
    while (!(SPI1->SR & SPI_SR_RXNE));

    // Read the data from SPI data register
    return (uint8_t)SPI1->DR;
}

uint8_t SPI1_TransmitReceive(uint8_t data) {
    // Wait until TXE flag is set
    while (!(SPI1->SR & SPI_SR_TXE));

    // Send data
    SPI1->DR = data;

    // Wait until RXNE flag is set
    while (!(SPI1->SR & SPI_SR_RXNE));

    // Return received data
    return (uint8_t)SPI1->DR;
}