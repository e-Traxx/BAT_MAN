#include "stm32f767xx.h"
#include "spi_handler.h"
#include <stdint.h>

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

struct Voltages {
    uint32_t Voltages[140];
} typedef Voltages ;

Voltages PollVoltage();

void ADBMS_INIT() {
    // SETUP the ADBMS SLAVES
}

// Send Read Voltage command
void ADBMSPoll() {
    // The ADBMSPoll function manages the polling of data from ADBMS and returns it out. 
   
    // Polling of Voltages

}

Voltages PollVoltage() {
    // 
}