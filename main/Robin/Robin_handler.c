#include "Robin_handler.h"
#include "assert.h"
#include "can_handler.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "spi_handler.h"
#include "string.h"
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
#define RDFLT 0xA800
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
Robin_container_t *robin;
// 6 Bytes (48 bits) per Module serial id.
// 6 Bytes in total * Max num of modules (NUM_STACKS)
uint8_t ADBMS_IDs[6 * NUM_STACKS];

// post Query value updates
void parse_voltages(SPI_responses_t *responses);
void parse_Temperature(SPI_responses_t *responses);

// ---- Setup ----
void ADBMS_Setup(void) {
  // Semaphore Setup
  adbms_semaphore = xSemaphoreCreateBinary();
  if (adbms_semaphore == NULL) {
    ESP_LOGE(TAG, "[-] Failed to initialise ADBMS Semaphore");
    return;
  }

  // Create Timer
  adbms_timer = xTimerCreate("ADBMS_Query_timer", pdMS_TO_TICKS(1000), pdTRUE,
                             (void *)0, Adbms_query_callback);
  if (adbms_timer == NULL) {
    ESP_LOGE(TAG, "[-] Failed to initialise ADBMS Timer");
    return;
  }

  // ID response
  uint8_t ID_Poll_resp[(6 * NUM_STACKS) + 2];

  // Poll All Devices and register their IDS
  adbms_broadcast_command((uint16_t *)RDSID, spi_cs1);
  isoSPI_receive(ID_Poll_resp, sizeof(ID_Poll_resp), spi_cs2);

  // Start Measuring
  adbms_broadcast_command((uint16_t *)ADCV, spi_cs1);
  adbms_broadcast_command((uint16_t *)OW_ALL, spi_cs1);
}

// Gives semaphore to query task every 200ms
void Adbms_query_callback(TimerHandle_t timer) {
  BaseType_t HighPriorityTaskWoken = pdFALSE;
  esp_err_t ret;

  // Give Semaphore to Query task
  ret = xSemaphoreGiveFromISR(adbms_semaphore, &HighPriorityTaskWoken);
  if (ret != pdPASS) {
    ESP_LOGE(TAG, "[-] %s Failed to give Semaphore to ADBMS query",
             esp_err_to_name(ret));
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

  while (1) {
    if (xSemaphoreTake(adbms_semaphore, portMAX_DELAY) == pdTRUE) {
      robin = malloc(sizeof(Robin_container_t));

      ESP_LOGV(TAG, "[+] Querying... \n");
      // First query the Cells
      //
      // 16-Bit values are stored in the uint8_t array
      // Therefore the number of elements is to be doubled
      // Where 16-Bits take up 2 spaces

      SPI_responses_t *responses_Volt =
          (SPI_responses_t *)malloc(sizeof(SPI_responses_t));
      SPI_responses_t *responses_Temp =
          (SPI_responses_t *)malloc(sizeof(SPI_responses_t));
      /* SPI_responses_t *redundant_responses_Volt = */
      /*     (SPI_responses_t *)malloc(sizeof(SPI_responses_t)); */
      /* SPI_responses_t *redundant_responses_Temp = */
      /*     (SPI_responses_t *)malloc(sizeof(SPI_responses_t)); */

      // Read all voltages
      adbms_broadcast_command((uint16_t *)RDFLT, spi_cs1);
      isoSPI_receive(responses_Volt->raw, sizeof(responses_Volt->raw), spi_cs2);

      // Read Temperature
      adbms_broadcast_command((uint16_t *)ADAX2, spi_cs1);
      isoSPI_receive(responses_Temp->raw, sizeof(responses_Temp->raw), spi_cs2);

      /* //---- Redundant Calls ----- */
      /* // Read all voltages */
      /* adbms_send_command(RDFLT, spi_cs1); */
      /* adbms_fetch_data(redundant_responses_Volt->raw, */
      /*                  sizeof(redundant_responses_Volt->raw), spi_cs2); */
      /**/
      /* // Read Temperature */
      /* adbms_send_command(ADAX2, spi_cs1); */
      /* adbms_fetch_data(redundant_responses_Temp->raw, */
      /*                  sizeof(redundant_responses_Temp->raw), spi_cs2); */
      /**/

      // ---- Parse and evaluate ----
      // Parse Cell voltages
      parse_voltages(responses_Volt);

      // Parse Temperature
      parse_Temperature(responses_Temp);

      // // ---- DUMMY CODE ----
      // /* // Then query the temperature (DUMMY) */
      // /* uint16_t voltage = 370; */
      // /* uint16_t temp = 750; */
      // /* // For now, we use a dumb algo */
      // /* for (int i = 0; i < 14; i++) { */
      // /*   for (int j = 0; j < 10; j++) { */
      // /*     robin->individual_voltages[i][j] = */
      // /*         voltage; // with factor 0.01 = 3.64V */
      // /*     robin->individual_temperatures[i][j] = temp; // with factor 0.1
      // /* = 75.0 */
      // /*   } */
      // /* } */

      System_report_to_user(robin);
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

void adbms_broadcast_command(uint16_t *cmd, spi_device_handle_t sender) {
  // Broadcasting (Poll Commands)

  // first load the Command into CM0 and CMD1
  uint8_t cmdWord[2];
  // The 16 Bit is split into 2 x 8 Bits
  cmdWord[0] = (*cmd >> 8) & 0xFF; // High Byte [8 - 15]
  cmdWord[1] = *cmd & 0xFF;        // Low Byte [0 - 7]

  // Calculate Command PEC
  uint16_t PEC = Compute_Command_PEC(cmdWord, sizeof(cmdWord));

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

void adbms_addressed_command(uint16_t *cmd, uint8_t *payload) {}

/// Poll the Devices for the serial_id and save the value in an array
/// This allows the device to identify the individual modules and use addressed
///
//
//
//
