#include "stm32f767xx.h"
#include "uart_handler.h"


USB_OTG_OUTEndpointTypeDef *out_ep0 = USB_OTG_FS_OUT(0); 
USB_OTG_INEndpointTypeDef *in_ep0 = USB_OTG_FS_IN(0);
void USB_INIT() {
    // * PAGE 164 REFERENCE
    // Clock and Pin Configuration 
    RCC->CR |= RCC_CR_HSEON; // Enable HSE (8MHz as Source Clock)
    while (!((RCC->CR) & RCC_CR_HSERDY)) {;}  // Wait until the Source clock is ready 

    // configure PLL 
    RCC-> PLLCFGR = (8 << RCC_PLLCFGR_PLLM_Pos)     // PLLM: 8
                |   (336 << RCC_PLLCFGR_PLLN_Pos)   // PLLN: 336
                |   (7 << RCC_PLLCFGR_PLLP_Pos)     // PLLP: 7
                |   (7 << RCC_PLLCFGR_PLLQ_Pos)     // PLLQ: 7 
                |   (RCC_PLLCFGR_PLLSRC_HSE);       // Use HSE as source
    
    RCC->CR |= RCC_CR_PLLON;
    while (!((RCC->CR) & RCC_CR_PLLRDY)) {;}

    // Set as system clock 
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);


    // USB Enable
    RCC->APB1ENR |= RCC_AHB2ENR_OTGFSEN;


    // Configure GPIO Pins
    // ENable GPIOA Clock access
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    // Configure PA11 (DM) and PA12 (DP) as AF10 (Alternate Function)  
    GPIOA->MODER &= ~(GPIO_MODER_MODER11 | GPIO_MODER_MODER12); // Clearing
    GPIOA->MODER |= (GPIO_MODER_MODER11_1 | GPIO_MODER_MODER12_1); // Setup AF mode


    GPIOA->AFR[1] |= (10 << (4 * (11 - 8))) | (10 << (4 * (12 - 8)));
    GPIOA->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR11 | GPIO_OSPEEDER_OSPEEDR12);

    // USB Core 
    USB_OTG_FS->GRSTCTL |= USB_OTG_GRSTCTL_CSRST; // CSRST (Core soft-reset)
    while (USB_OTG_FS->GRSTCTL & USB_OTG_GRSTCTL_CSRST){;};

     // Interrupts
     USB_OTG_FS->GUSBCFG = 0; // Disable all initial interrupts 
     USB_OTG_FS->GUSBCFG |= USB_OTG_GAHBCFG_GINT; // Enable Global INTerrupt mask

     // Select Device Mode (Device Mode)
     USB_OTG_FS->GUSBCFG &= ~USB_OTG_GUSBCFG_FHMOD; // Disable Host Mode
     USB_OTG_FS->GUSBCFG |= USB_OTG_GUSBCFG_FDMOD; // Enable Device MOde

    // !DELAY

    USB_OTG_FS_DEVICE->DCFG &= ~USB_OTG_DCFG_DSPD;
    USB_OTG_FS_DEVICE->DCFG |= USB_OTG_DCFG_DSPD_0;


    // Enable USBFS Global Interrupt 
    NVIC_EnableIRQ(OTG_FS_IRQn);
    NVIC_SetPriority(OTG_FS_IRQn, 0);

    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_USBRST;      // USB reset interrupt
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_ENUMDNEM;    // Enumeration done interrupt
    USB_OTG_FS->GINTMSK |= USB_OTG_GINTMSK_RXFLVLM;     // Rx FIFO non-empty interrupt
    

    // Configure Endpoints
    USB_OTG_FS_DEVICE->DAINTMSK |= (1 << 0); // Unmask EP0 OUT and IN interrupts

  // Access EP0
    // Clear the MPSIZ bits (bits 0-1 for EP0 max packet size)
    in_ep0->DIEPCTL &= ~(3 << 0);  // Clear bits 0-1
    // Set MPSIZ for 64 bytes (0b00 corresponds to 64 bytes)
    in_ep0->DIEPCTL |= (0 << 0);   // Set max packet size to 64 bytes (64 bytes is 0b00)
    in_ep0->DIEPCTL |= USB_OTG_DIEPCTL_SNAK;

 // Access EP0
    // Clear the MPSIZ bits (bits 0-1 for EP0 max packet size)
    out_ep0->DOEPCTL &= ~(3 << 0);  // Clear bits 0-1
    // Set MPSIZ for 64 bytes (0b00 corresponds to 64 bytes)
    out_ep0->DOEPCTL |= (0 << 0);   // Set max packet size to 64 bytes (64 bytes is 0b00)
    out_ep0->DOEPCTL |= USB_OTG_DOEPCTL_SNAK;
}


void OTG_FS_IRQHandler() {
    U_debug("USB Interrupt");

    uint8_t gintsts = USB_OTG_FS->GINTSTS;
    U_debug("HELLO");

    if (gintsts & USB_OTG_GINTSTS_USBRST) {
        // Handle USB Reset event
        USB_OTG_FS->GINTSTS |= USB_OTG_GINTSTS_USBRST;  // Clear the reset interrupt
        // Reinitialize the device, endpoints, etc.
    }

    if (gintsts & USB_OTG_GINTSTS_ENUMDNE) {
        // Handle Enumeration Done event
        USB_OTG_FS->GINTSTS |= USB_OTG_GINTSTS_ENUMDNE;  // Clear the enumeration done interrupt
        // Setup endpoints and prepare for data transfer
    }

    if (gintsts & USB_OTG_GINTSTS_RXFLVL) {
        // Handle Rx FIFO non-empty event
        USB_OTG_FS->GINTSTS |= USB_OTG_GINTSTS_RXFLVL;  // Clear Rx FIFO interrupt
        // Read and process the received data
    }


    if (out_ep0->DOEPINT & USB_OTG_DOEPINT_STUP) {
        U_debug("Sent something");
        // Read setup packet
        uint32_t setupPacket[2];
            // Clear interrupt

            // Handle setup packet
        // ...
    }
}











// #include "usb_handler.h"
// #include "Driver_Common.h"
// #include "Driver_USBD.h"
// #include <cstdint>

// // USBD Driver 
// extern ARM_DRIVER_USBD Driver_USBD;

// void USBD_callback(uint32_t event) {

// }

// void USBD_ENDPOINT_callback(uint8_t ep_addr, uint32_t event) {

// }

// void USB_INIT() {
//     static ARM_DRIVER_USBD* USBdrv = &Driver_USBD;
//     ARM_DRIVER_VERSION version;
//     ARM_USBD_CAPABILITIES drv_capabilizies;

//     // Initialise and provide Power to GPIO
//     USBdrv->Initialize(USBD_callback, USBD_ENDPOINT_callback);
//     USBdrv->PowerControl(ARM_POWER_FULL); // Power up Peripheral

//     // Configuration


// }