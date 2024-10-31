#include "cmsis_os2.h"
#include "stm32f767xx.h"
#include "spi_handler.h"
#include <stdint.h>

#define initialise_JOB 0x0001
#define Query_JOB 0x0002
#define System_Check_JOB 0x0003

// Every Robin is a slave module
struct Robin {
    uint8_t Voltages[14]; // 14 Zellen Voltages 
    uint8_t Temperatures[14]; // 14 Temperature values 
} typedef Robin;

 /*
    * First: Polling of all voltage Measurements from the ADBMS6830 and saving the values in a Struct
    * 2nd: Send the Polling commands for Temperature pass 1 to 7-8
    * 3rd: In the meantime, the Voltage values are worked on. 
    * 4th: The Temperature values are worked on
    * Start from 1
    */

    /*
    *  HOW DOES IT WORK??
    *  We have 2 Threads: 
    *      1. SPI Polling Thread -> manages the polling
    *      2. Data Processing Thread -> Processes the data
*/

// Siegfried is der akku 
Robin Siegfried [10]; // The Accumulator has a configuration of 14 x 10 cells. 10 x (14 cell monitoring Robin module)
void PollVoltage();

void ADBMS_MAIN() {
    // SETUP the ADBMS SLAVES
    for (;;) {
        uint32_t flags = osThreadFlagsWait(initialise_JOB | Query_JOB | System_Check_JOB, osFlagsWaitAny, osWaitForever);

        // Initialise JOB
        if (flags & initialise_JOB) {
            // Set Continuous measurement mode C-ADC
            SPI1_Transmit((uint8_t) 0x5C0);
            
        }   

        if (flags & Query_JOB) {

        } 

        if (System_Check_JOB) {

        }


    }
}

// Send Read Voltage command
void ADBMSPoll() {
    // The ADBMSPoll function manages the polling of data from ADBMS and returns it out. 
    
    // Polling of Voltages

}

void PollVoltage() {
    
}