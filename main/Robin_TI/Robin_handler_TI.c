#include "Robin_TI_handler.h"
#include "assert.h"
#include "can_handler.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

// !! Things that need to be Changed:
//  setup
//  query
//  preprocesscmd
//  broadcast
//  read_command

/// This is a fucking Hell hole.
///
/// This piece of shit handles the surface level operation of the Communication
/// between the Master and the Slaves The rest are handled by the other files.
///
/// Robin_handler: CAN and SPI Communication
/// Robin_Balancing: Balancing logic and Payload manufacturing for Communication
/// Robin_parser: Parses the received data and loads it up in Memory
///

// Command Container
uint16_t CMD;
SemaphoreHandle_t adbms_semaphore;
TimerHandle_t adbms_timer;

static const char *TAG = "\nADBMS_Query";

static SPI_responses_t rx_volt DMA_ATTR;
static SPI_responses_t rx_temp DMA_ATTR;

// Function defintion
void Adbms_query_callback (TimerHandle_t adbms_timer);
void Robin_query (void *args);
void System_report_to_user ();
void adbms_broadcast_command (uint16_t *cmd, spi_device_handle_t sender);
void adbms_addressed_command (uint16_t *cmd, uint8_t *payload);
void adbms_read_command (uint16_t *cmd, uint8_t *responses, size_t response_len, spi_device_handle_t sender);

// Vlaue Container
Robin_container_t robin_ptr;
Robin_container_t *robin = &robin_ptr;
// 6 Bytes (48 bits) per Module serial id.
// 6 Bytes in total * Max num of modules (NUM_STACKS)
uint8_t ADBMS_IDs[6 * NUM_STACKS];

// ---- Setup ----
void
ADBMS_Setup (void)
{
    // Semaphore Setup
    adbms_semaphore = xSemaphoreCreateBinary ();
    if (adbms_semaphore == NULL)
	{
	    ESP_LOGE (TAG, "[-] Failed to initialise ADBMS Semaphore");
	    return;
	}

    // Create Timer - Changed from 200ms to 500ms
    adbms_timer = xTimerCreate ("ADBMS_Query_timer", pdMS_TO_TICKS (1000), pdTRUE, (void *)0, Adbms_query_callback);
    if (adbms_timer == NULL)
	{
	    ESP_LOGE (TAG, "[-] Failed to initialise ADBMS Timer");
	    return;
	}

    // ID response
    // uint8_t ID_Poll_resp[(6 * NUM_STACKS) + 2];
    // CMD = RDFLT;
    //
    // Poll All Devices and register their IDS
    // adbms_read_command(&CMD, ID_Poll_resp, sizeof(ID_Poll_resp), spi_cs1);

    // Start Measuring
    CMD = ADCV;
    ESP_LOGV (TAG, "[+] Start Measuring (ADCV Command)");
    adbms_broadcast_command (&CMD, spi_cs1);
}

// Gives semaphore to query task every 200ms
void
Adbms_query_callback (TimerHandle_t timer)
{
    BaseType_t HighPriorityTaskWoken = pdFALSE;

    // Give Semaphore to Query task - simplified version
    xSemaphoreGiveFromISR (adbms_semaphore, &HighPriorityTaskWoken);

    // If a task was woken, we should yield
    if (HighPriorityTaskWoken)
	{
	    portYIELD_FROM_ISR ();
	}
}

/*
 *
 * Communication Side
 *
 * SPI -> Communication with 14 Slave modules
 *     * Report Cell voltage Values
 *     * Report Temperature Values
 *
 * CAN -> Communication with ECU and User laptop as well as Charger
 *     * Report individual_voltages and temperature to User
 *     * Report Diagnostic data to ECU for latching and display
 *
 */

// Query the Slave modules
//
// Procedure:
//  1. Poll the Cell modules
//  2. Parse the values from the modules
//  3. Run Diagnostic checks on connections
//  4. Run Diagnostic checks on the values
//  5. Check Integrity of System
//  6. Run OCV and Coulomb counting to determine
//  SOC
//
void
Robin_query (void *args)
{
    UBaseType_t uxHighWaterMark;

    while (1)
	{
	    // Wait for semaphore with a timeout
	    if (xSemaphoreTake (adbms_semaphore, pdMS_TO_TICKS (1000)) == pdTRUE)
		{
		    // Get stack high water mark
		    uxHighWaterMark = uxTaskGetStackHighWaterMark (NULL);
		    // ESP_LOGI(TAG, "Stack high water mark: %u", uxHighWaterMark);

		    ESP_LOGV (TAG, "[+] Querying... \n");

		    uint8_t raw[8];
		    CMD = RDSID;
		    ESP_LOGI (TAG, "[~] RDSID Command");
		    adbms_read_command (&CMD, raw, sizeof (raw), spi_cs1);

		    // // Read all voltages
		    // CMD = RDFLT;
		    // ESP_LOGI(TAG, "[~] RDFLT Command");
		    // adbms_read_command(&CMD, rx_volt.raw, sizeof(rx_volt.raw), spi_cs1);
		    //
		    // Read Temperature
		    // CMD = RDSID;
		    // ESP_LOGI(TAG, "[~] ADAX2 Command");
		    // adbms_read_command(&CMD, rx_temp.raw, sizeof(rx_temp.raw), spi_cs1);
		    //
		    // ---- Parse and evaluate ----
		    // Parse Cell voltages
		    parse_voltages (rx_volt.raw);
		    // Parse Temperature
		    parse_temperatures (rx_volt.raw);

		    // Report to user
		    // System_report_to_user();
		}
	}
}

// CAN TX
/*
 * Report Values through CAN to the ECU and Laptop interface
 *
 */
void
System_report_to_user ()
{
    individual_temperatures_frame_t Tframe;
    individual_voltages_frame_t Vframe;

    // 28 Muxes (5 Values per Mux)
    // 1 -28
    // 0x01 - 0x1C
    uint16_t first_half[5];
    uint16_t second_half[5];
    for (int mux = 1; mux < 29; mux++)
	{
	    // for every pack/ stacks
	    for (int pack = 0; pack < 14; pack++)
		{
		    // take 5 values interval
		    //
		    // for 10 in a stack
		    for (int i = 0; i < 5; i++)
			{
			    first_half[i] = robin->individual_voltages[pack][i];
			    second_half[i] = robin->individual_voltages[pack][i + 5];
			}
		    // send first 5
		    individual_voltages_formatter (&Vframe, first_half, mux);
		    CAN_TX_enqueue (0x202, 8, Vframe.bytes);
		    // send last 5
		    individual_voltages_formatter (&Vframe, second_half, mux);
		    CAN_TX_enqueue (0x202, 8, Vframe.bytes);

		    // now Temperatures
		    // take 5 values interval
		    for (int i = 0; i < 5; i++)
			{
			    first_half[i] = robin->individual_temperatures[pack][i];
			    second_half[i] = robin->individual_temperatures[pack][i + 5];
			}
		    // send first 5
		    individual_temperatures_formatter (&Tframe, first_half, mux);
		    CAN_TX_enqueue (0x100, 8, Tframe.bytes);
		    // send last 5
		    individual_temperatures_formatter (&Tframe, second_half, mux);
		    CAN_TX_enqueue (0x100, 8, Tframe.bytes);
		}
	}
}

// SPI TX and RX

// Procedure:
//  1. Check the PEC of the response packet
//  2. Isolate values per Stack
//  3. Update the values in Robin container
//

// SPI TX
//
// cmdCode: Command itself
//
// Procedure: (BATMAN)
// 1. Calculate the 15 Bit Command PEC to be added at the end of the Command
// 2. Load the Command and the PEC into the Buffer using big Endian
// 3. Transmit the data to the slaves
//
// Procedure: (ROBIN)
// 1. The first Robin gets the Command and sends it forward to the next device
// in the chain until the data reaches the Port B.
// 2. In the meantime, the master is clocking dummy data, onto which the modules
// load their data on.

void
preprocess_cmd (uint16_t *cmd, uint8_t *cmdWord, uint16_t *PEC)
{
    // Defensive Programming
    if (cmd == NULL)
	{
	    ESP_LOGE (TAG, "[-] Pointer is NULL");
	    return;
	}

    // first load the Command into CM0 and CMD1
    // The 16 Bit is split into 2 x 8 Bits
    cmdWord[0] = (*cmd >> 8) & 0xFF; // High Byte [8 - 15]
    cmdWord[1] = *cmd & 0xFF;	     // Low Byte [0 - 7]

    // Calculate Command PEC
    *PEC = pec15_calc (cmdWord, 2);
}

// Only send and no Read
void
adbms_broadcast_command (uint16_t *cmd, spi_device_handle_t sender)
{
    // Broadcasting (Poll Commands)

    // Defensive Programming
    if (cmd == NULL)
	{
	    ESP_LOGE (TAG, "[-] Pointer is NULL");
	    return;
	}

    // first load the Command into CM0 and CMD1
    uint8_t cmdWord[2];
    // Command PEC
    uint16_t PEC;

    preprocess_cmd (cmd, cmdWord, &PEC);

    // Load the command Word (CMD0 and CMD1) and PEC into the transmission array
    uint8_t transmission_data[4] = { cmdWord[0], cmdWord[1], ((PEC >> 8) & 0xFF), (PEC & 0xFF) };

    // Transmit using SPI
    isoSPI_tx_rx (sender, transmission_data, sizeof (transmission_data), NULL, 0);
}

// cmdCode: Command itself
// payload: the N x Modules
//
// Procedure: (BATMAN)
// 1. Calculate the 15 Bit Command PEC to be added at the end of the Command
// 2. Load the Command and the PEC into the Buffer using big Endian
// 3. Transmit the data to the slaves
//
// Procedure: (ROBIN)
// 1. The first Robin gets the Command and sends it forward to the next device
// in the chain until the data reaches the Port B.
// 2. In the meantime, the master is clocking dummy data, onto which the modules
// load their data on.

// void adbms_addressed_command(uint16_t *cmd, uint8_t *payload) {
//   // Send serialised data prepended with a Command
//   // The Serialised Data contains the payload or bit order which is going to
//   be
//   // used by the ADBMS6830 to determine which cells are going to be balanced
//   or
//   // not.
//
//   if (cmd == NULL) {
//     ESP_LOGE(TAG, "[-] CMD Pointer is NULL");
//     return;
//   }
//
//   // first load the Command into CM0 and CMD1
//   uint8_t cmdWord[2];
//   // Command PEC
//   uint16_t PEC;
//
//   preprocess_cmd(cmd, cmdWord, &PEC);
// }
//
void
adbms_read_command (uint16_t *cmd, uint8_t *responses, size_t response_len, spi_device_handle_t sender)
{
    // isoSPI_receive(responses_Volt->raw, sizeof(responses_Volt->raw),

    if (cmd == NULL)
	{
	    ESP_LOGE (TAG, "[-] CMD Pointer is NULL");
	    return;
	}

    // first load Command into CMD0 and CMD1
    uint8_t cmdWord[2];
    // Command PEC
    uint16_t PEC;

    preprocess_cmd (cmd, cmdWord, &PEC);
    // // Load Transmission Data
    // static uint8_t transmission_data[4] = {cmdWord[0], cmdWord[1],
    //                                        ((PEC >> 8) & 0xFF), (PEC & 0xFF)};
    static uint8_t transmission_data[4];
    transmission_data[0] = cmdWord[0];
    transmission_data[1] = cmdWord[1];
    transmission_data[2] = (PEC >> 8) & 0xFF;
    transmission_data[3] = (PEC & 0xFF);
    ESP_LOGI ("ADBMS_CMD", "Command: 0x%04X, PEC: 0x%04X", *cmd, PEC);
    ESP_LOGE ("ADBMS_CMD", "TX: [%02X %02X %02X %02X]", transmission_data[0], transmission_data[1],
	      transmission_data[2], transmission_data[3]);
    // Send and Read
    isoSPI_tx_rx (sender, transmission_data, sizeof (transmission_data), responses, response_len);
}
