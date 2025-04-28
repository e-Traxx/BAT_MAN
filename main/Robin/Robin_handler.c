#include "Robin_handler.h"
#include "assert.h"
#include "can_handler.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "spi_handler.h"
#include "string.h"
#include "Robin_types.h"
#include <stddef.h>
#include <stdint.h>

/// This is a fucking Hell hole.
///
/// This piece of shit handles the surface level operation of the Communication
/// between the Master and the Slaves The rest are handled by the other files.
///
/// Robin_handler: CAN and SPI Communication
/// Robin_Balancing: Balancing logic and Payload manufacturing for Communication
/// Robin_parser: Parses the received data and loads it up in Memory
///

// Commands
// Reads all the Filtered Cell Voltage values
#define RDFLT 0b0000001111110110
// Start continous Cell voltage conversion
#define ADCV 0b0000001111110110
// Start Continous Redundant Voltage Conversion
#define ADSV 0b0000000111111001
// Start AUX ADC continous voltage conversion
#define ADAX2 0b000001000000000
// Start Open Wire Detection for all Cells
#define OW_ALL 0b0000000101110000
// Poll devices for serial_id
#define RDSID 0b0000000000101100

// Command Container
uint16_t CMD;
SemaphoreHandle_t adbms_semaphore;
TimerHandle_t adbms_timer;

static const char *TAG = "ADBMS_Query";

// Function defintion
void Adbms_query_callback(TimerHandle_t adbms_timer);
void Robin_query(void *args);
void System_report_to_user(Robin_container_t *robin);
void individual_voltages_formatter(individual_voltages_frame_t *frame,
                                   uint16_t voltages[5], uint8_t mux);
void individual_temperatures_formatter(individual_temperatures_frame_t *frame,
                                       uint16_t temps[5], uint8_t mux);
uint16_t extract_received_pec(uint16_t *rxbuffer);
/* void adbms_send_command(uint8_t address, bool Broadcast, uint16_t cmdCode, */
/* const uint16_t *payload, size_t payloadLen); */
void adbms_broadcast_command(uint16_t *cmd, spi_device_handle_t sender);
void adbms_addressed_command(uint16_t *cmd, uint8_t *payload);

// Vlaue Container
Robin_container_t robin_ptr;
Robin_container_t *robin = &robin_ptr;
// 6 Bytes (48 bits) per Module serial id.
// 6 Bytes in total * Max num of modules (NUM_STACKS)
uint8_t ADBMS_IDs[6 * NUM_STACKS];

// post Query value updates
parser_error_t parse_voltages(SPI_responses_t *responses);
parser_error_t parse_Temperature(SPI_responses_t *responses);

// ---- Setup ----
void ADBMS_Setup(void) {
  // Semaphore Setup
  adbms_semaphore = xSemaphoreCreateBinary();
  if (adbms_semaphore == NULL) {
    ESP_LOGE(TAG, "[-] Failed to initialise ADBMS Semaphore");
    return;
  }

  // Create Timer - Changed from 200ms to 500ms
  adbms_timer = xTimerCreate("ADBMS_Query_timer", pdMS_TO_TICKS(500), pdTRUE,
                             (void *)0, Adbms_query_callback);
  if (adbms_timer == NULL) {
    ESP_LOGE(TAG, "[-] Failed to initialise ADBMS Timer");
    return;
  }

  // ID response
  uint8_t ID_Poll_resp[(6 * NUM_STACKS) + 2];
  CMD = RDFLT;

  // Poll All Devices and register their IDS
  adbms_broadcast_command(&CMD, spi_cs1);
  isoSPI_receive(ID_Poll_resp, sizeof(ID_Poll_resp), spi_cs2);

  // Start Measuring
  CMD = ADCV;
  adbms_broadcast_command(&CMD, spi_cs1);
  adbms_broadcast_command(&CMD, spi_cs1);
}

// Gives semaphore to query task every 200ms
void Adbms_query_callback(TimerHandle_t timer) {
  BaseType_t HighPriorityTaskWoken = pdFALSE;
  
  // Give Semaphore to Query task - simplified version
  xSemaphoreGiveFromISR(adbms_semaphore, &HighPriorityTaskWoken);
  
  // If a task was woken, we should yield
  if (HighPriorityTaskWoken) {
    portYIELD_FROM_ISR();
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
void Robin_query(void *args) {
  UBaseType_t uxHighWaterMark;
  
  while (1) {
    // Wait for semaphore with a timeout
    if (xSemaphoreTake(adbms_semaphore, pdMS_TO_TICKS(1000)) == pdTRUE) {
      // Get stack high water mark
      uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      ESP_LOGI(TAG, "Stack high water mark: %u", uxHighWaterMark);
      
      // Allocate memory for this iteration
      Robin_container_t *local_robin = malloc(sizeof(Robin_container_t));
      if (local_robin == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for Robin container");
        continue;
      }

      ESP_LOGV(TAG, "[+] Querying... \n");
      
      // Allocate memory for responses
      SPI_responses_t *responses_Volt = malloc(sizeof(SPI_responses_t));
      SPI_responses_t *responses_Temp = malloc(sizeof(SPI_responses_t));
      
      if (responses_Volt == NULL || responses_Temp == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory for responses");
        free(local_robin);
        if (responses_Volt) free(responses_Volt);
        if (responses_Temp) free(responses_Temp);
        continue;
      }

      // Read all voltages
      CMD = RDFLT;
      adbms_broadcast_command(&CMD, spi_cs1);
      isoSPI_receive(responses_Volt->raw, sizeof(responses_Volt->raw), spi_cs2);

      // Read Temperature
      CMD = ADAX2;
      adbms_broadcast_command(&CMD, spi_cs1);
      isoSPI_receive(responses_Temp->raw, sizeof(responses_Temp->raw), spi_cs2);

      // ---- Parse and evaluate ----
      // Parse Cell voltages
      parse_voltages(responses_Volt);

      // Parse Temperature
      parse_Temperature(responses_Temp);

      // Report to user
      System_report_to_user(local_robin);

      // Free allocated memory
      free(responses_Volt);
      free(responses_Temp);
      free(local_robin);
    }
  }
}

// CAN TX
/*
 * Report Values through CAN to the ECU and Laptop interface
 *
 */
void System_report_to_user(Robin_container_t *robin) {
  individual_temperatures_frame_t Tframe;
  individual_voltages_frame_t Vframe;

  // 28 Muxes (5 Values per Mux)
  // 1 -28
  // 0x01 - 0x1C
  uint16_t first_half[5];
  uint16_t second_half[5];
  for (int mux = 1; mux < 29; mux++) {
    // for every pack/ stacks
    for (int pack = 0; pack < 14; pack++) {
      // take 5 values interval
      //
      // for 10 in a stack
      for (int i = 0; i < 5; i++) {
        first_half[i] = robin->individual_voltages[pack][i];
        second_half[i] = robin->individual_voltages[pack][i + 5];
      }
      // send first 5
      individual_voltages_formatter(&Vframe, first_half, mux);
      CAN_TX_enqueue(0x202, 8, Vframe.bytes);
      // send last 5
      individual_voltages_formatter(&Vframe, second_half, mux);
      CAN_TX_enqueue(0x202, 8, Vframe.bytes);

      // now Temperatures
      // take 5 values interval
      for (int i = 0; i < 5; i++) {
        first_half[i] = robin->individual_temperatures[pack][i];
        second_half[i] = robin->individual_temperatures[pack][i + 5];
      }
      // send first 5
      individual_temperatures_formatter(&Tframe, first_half, mux);
      CAN_TX_enqueue(0x100, 8, Tframe.bytes);
      // send last 5
      individual_temperatures_formatter(&Tframe, second_half, mux);
      CAN_TX_enqueue(0x100, 8, Tframe.bytes);
    }
  }

  free(robin);
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

void preprocess_cmd(uint16_t *cmd, uint8_t *cmdWord, uint16_t *PEC) {
  // Defensive Programming
  if (cmd == NULL) {
    ESP_LOGE(TAG, "[-] Pointer is NULL");
    return;
  }

  // first load the Command into CM0 and CMD1
  // The 16 Bit is split into 2 x 8 Bits
  cmdWord[0] = (*cmd >> 8) & 0xFF; // High Byte [8 - 15]
  cmdWord[1] = *cmd & 0xFF;        // Low Byte [0 - 7]

  // Calculate Command PEC
  *PEC = Compute_Command_PEC(cmdWord, sizeof(cmdWord));
}

void adbms_broadcast_command(uint16_t *cmd, spi_device_handle_t sender) {
  // Broadcasting (Poll Commands)

  // Defensive Programming
  if (cmd == NULL) {
    ESP_LOGE(TAG, "[-] Pointer is NULL");
    return;
  }

  // first load the Command into CM0 and CMD1
  uint8_t cmdWord[2];
  // Command PEC
  uint16_t PEC;

  preprocess_cmd(cmd, cmdWord, &PEC);

  // Load the command Word (CMD0 and CMD1) and PEC into the transmission array
  uint8_t transmission_data[4] = {cmdWord[0], cmdWord[1], ((PEC >> 8) & 0xFF),
                                  (PEC & 0xFF)};

  // Transmit using SPI 1
  isoSPI_transmit(transmission_data, sizeof(transmission_data), sender);
}

// cmdCode: Command itself
// payload: the N x
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

void adbms_addressed_command(uint16_t *cmd, uint8_t *payload) {
  // Send serialised data prepended with a Command
  // The Serialised Data contains the payload or bit order which is going to be
  // used by the ADBMS6830 to determine which cells are going to be balanced or
  // not.

  if (cmd == NULL) {
    ESP_LOGE(TAG, "[-] CMD Pointer is NULL");
    return;
  }

  // first load the Command into CM0 and CMD1
  uint8_t cmdWord[2];
  // Command PEC
  uint16_t PEC;

  preprocess_cmd(cmd, cmdWord, &PEC);
}
