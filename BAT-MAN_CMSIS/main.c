#include "RTE_Components.h"
#include "uart_handler.h"

#include CMSIS_device_header
#include "stm32f767xx.h"  // CMSIS header file for STM32F767


int main(void) {
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

    // initialise UART
    UART_INIT();

    while (1) {

        U_debug("Hello World!");
		for(volatile int i=0;i<100000;i++);
    }
}