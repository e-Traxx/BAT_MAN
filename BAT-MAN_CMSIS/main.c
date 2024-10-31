#include "uart_handler.h"
#include "stm32f767xx.h"
#include "can_handler.h"
#include "spi_handler.h"
#include "cmsis_os2.h"
#include "adbms_handler.h"
#include "current_handler.h"



// Threads Definition
osThreadId_t adbms_query; // manages the quering of the slave boards
osThreadId_t CAN_Rx;    // Manages the receiving of data and bearbeitung
osThreadId_t CAN_Tx;    // Manages the formating and sending of data
osThreadId_t Curr;  // Current Sensor ADC
osThreadId_t Error_management;    
osThreadId_t Initialisation; // Initialises the system and does a system check and deletes it
osThreadId_t System_check; // runs a system check at regular intervals and reports it

// SubJobs
osThreadId_t Process_msg;
osThreadId_t Voltage_query;
osThreadId_t Temp_query;
osThreadId_t Status_query;
osThreadId_t Balancing_query;
osThreadId_t Error_set; // Set the Error Status for the appropriate error


// Initialises the System and does a system check.
void Initialise_system(void *arguments) {
    // TODO: Check SPI 
    // TODO: Check CAN
    // TODO: setup sequence 
    // TODO: System Checks setup

    // Check SPI
    // osThreadFlagsSet(adbms_query, 0x0001); // Starting check
    // Check CAN
    osThreadFlagsSet(CAN_Tx, 0x0001);
    // Check Current Sensor
    osThreadFlagsSet(Curr, 0x0001);

    // Send Diagnostics to ECU and CAN Check
    
    

}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start Kernel
 *---------------------------------------------------------------------------*/
void app_main(void *arguments) {
    // Main Jobs
    // adbms_query = osThreadNew(ADBMS_MAIN, NULL, NULL);
    // CAN_Rx = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
    // CAN_Tx = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
    Curr = osThreadNew(Curr_main, NULL, NULL);
    Initialisation = osThreadNew(Initialise_system, NULL, NULL);

    // Query subThreads
    // Voltage_query = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
    // Temp_query = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
    // Status_query = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
    // Balancing_query = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
    // Error_set = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);
    // Process_msg = osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr);

    // set starting Task
    osThreadFlagsSet(Initialisation, 0x0001); // Set signal to Initialisation 
    
    osDelay(osWaitForever);
}


int main(void) {
    
    // initialise Peripherals
    UART_INIT();
    CAN1_INIT();
    SPI_INIT();

    // Initialise CMSIS RTOS
    SystemCoreClockUpdate();    // System Initialisation
    osKernelInitialize();   // Start CMSIS-RTOS
    
    // // Message Queues for Data transfer between Threads 
    // osMessageQueueId_t data_queue_id = osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, NULL);
    // osMessageQueueId_t can_queue_id = osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, NULL);

    osThreadNew(app_main, NULL, NULL);
    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }


    while (1) {
    }
}

