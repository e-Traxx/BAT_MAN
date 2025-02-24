#include "Adbms_handler.h"
#include "assert.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "include/can_handler.h"
#include "include/spi_handler.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

// Commands
// Reads all the Filtered Cell Voltage values
#define RDFLT 0xA800
// Start continous Cell voltage conversion
#define ADCV 0b01111110110
// Start Continous Redundant Voltage Conversion
#define ADSV 0b00111111001
// Start AUX ADC continous voltage conversion
#define ADAX2 0b1000000000
// Start Open Wire Detection for all Cells
#define OW_ALL 0b00101110000
// Poll devices for serial_id
#define RDSID 0b00000101100

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
uint8_t calculate_PEC(const uint8_t *data, size_t len);
void Read_Voltage(uint8_t *responses_A, uint8_t *responses_B);
void register_devices(uint8_t *ID_response);
uint16_t extract_received_pec(uint16_t *rxbuffer);
void adbms_send_command(uint8_t address, bool Broadcast, uint16_t cmdCode,
                        const uint16_t *payload, size_t payloadLen);

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
  adbms_send_command(RDSID, spi_cs1);
  adbms_fetch_data(ID_Poll_resp, sizeof(ID_Poll_resp), spi_cs2);

  // Start Measuring
  adbms_send_command(ADCV, spi_cs1);
  adbms_send_command(OW_ALL, spi_cs1);
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
//  6. Run OCV and Coulomb counting and potentially Kalman filter to determine
//  SOC and SOH
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

      /* uint8_t responses_Volt[(NUM_STACKS + 1) * 2]; */
      /* uint8_t responses_Temp[(NUM_STACKS + 1) * 2]; */
      /**/
      /* uint8_t redundant_responses_Volt[(NUM_STACKS + 1) * 2]; */
      /* uint8_t redundant_responses_Temp[(NUM_STACKS + 1) * 2]; */
      /**/

      /* uint8_t *responses_Volt_t = */
      /*     (uint8_t *)malloc(((NUM_STACKS + 1) * 2) * sizeof(uint8_t)); */
      /* uint8_t *responses_Temp_t = */
      /*     (uint8_t *)malloc(((NUM_STACKS + 1) * 2) * sizeof(uint8_t)); */
      /**/
      /* uint8_t *redundant_responses_Volt_t = */
      /*     (uint8_t *)malloc(((NUM_STACKS + 1) * 2) * sizeof(uint8_t)); */
      /* uint8_t *redundant_responses_Temp_t = */
      /*     (uint8_t *)malloc(((NUM_STACKS + 1) * 2) * sizeof(uint8_t)); */
      /**/
      /* uint16_t responses_Volt[(NUM_STACKS + 1)]; */
      /* uint16_t responses_Temp[(NUM_STACKS + 1)]; */
      /**/

      SPI_responses_t *responses_Volt =
          (SPI_responses_t *)malloc(sizeof(SPI_responses_t));
      SPI_responses_t *responses_Temp =
          (SPI_responses_t *)malloc(sizeof(SPI_responses_t));
      SPI_responses_t *redundant_responses_Volt =
          (SPI_responses_t *)malloc(sizeof(SPI_responses_t));
      SPI_responses_t *redundant_responses_Temp =
          (SPI_responses_t *)malloc(sizeof(SPI_responses_t));

      // Read all voltages
      adbms_send_command(RDFLT, spi_cs1);
      adbms_fetch_data(responses_Volt->raw, sizeof(responses_Volt->raw),
                       spi_cs2);

      // Read Temperature
      adbms_send_command(ADAX2, spi_cs1);
      adbms_fetch_data(responses_Temp->raw, sizeof(responses_Temp->raw),
                       spi_cs2);

      //---- Redundant Calls -----
      // Read all voltages
      adbms_send_command(RDFLT, spi_cs1);
      adbms_fetch_data(redundant_responses_Volt->raw,
                       sizeof(redundant_responses_Volt->raw), spi_cs2);

      // Read Temperature
      adbms_send_command(ADAX2, spi_cs1);
      adbms_fetch_data(redundant_responses_Temp->raw,
                       sizeof(redundant_responses_Temp->raw), spi_cs2);

      // ---- Parse and evaluate ----
      // Parse Cell voltages
      parse_voltages(responses_Volt);

      // Parse Temperature
      parse_Temperature(responses_Temp);

      /* */
      /* SOC and SOH */

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

/*
 *
 * Parsing Data for and from Communication
 *
 */

void individual_voltages_formatter(individual_voltages_frame_t *frame,
                                   uint16_t voltages[5], uint8_t mux) {
  memset(frame->bytes, 0, sizeof(frame->bytes));

  frame->fields.mux = mux;
  frame->fields.voltage_1 = voltages[0] & 0x3FF;
  frame->fields.voltage_2 = voltages[1] & 0x3FF;
  frame->fields.voltage_3 = voltages[2] & 0x3FF;
  frame->fields.voltage_4 = voltages[3] & 0x3FF;
  frame->fields.voltage_5 = voltages[4] & 0x3FF;

  frame->fields.Reserved = 0;
}

void individual_temperatures_formatter(individual_temperatures_frame_t *frame,
                                       uint16_t temps[5], uint8_t mux) {
  frame->fields.mux = mux;
  frame->fields.temp_1 = temps[0] & 0x3FF;
  frame->fields.temp_2 = temps[1] & 0x3FF;
  frame->fields.temp_3 = temps[2] & 0x3FF;
  frame->fields.temp_4 = temps[3] & 0x3FF;
  frame->fields.temp_5 = temps[4] & 0x3FF;

  frame->fields.Reserved = 0;
}

// SPI TX and RX

// Procedure:
//  1. Check the PEC of the response packet
//  2. Isolate values per Stack
//  3. Update the values in Robin container
//

// SPI TX
//
// address: Address of the Module
// Broadcast: addressed or Broadcasted command
// cmdCode: Command itself
// Payload: Value to send
void adbms_send_command(uint8_t address, bool Broadcast, uint16_t cmdCode,
                        const uint16_t *payload, size_t payloadLen) {
  // Construct 16-bit Command Word
  // Check if Addressed transmission or Broadcasted
  uint16_t commmandWord = 0;
  if (Broadcast) {
    //[10] => ignore address
    commmandWord = (1u << 10) | (cmdCode & 0x3FF); // 10 Bit
  } else {
    // [10]=0 => addressed, [9:0]=cmdCode
    commmandWord = ((address & 0x1F) << 11 | (cmdCode & 0x3FF));
  }

  // Compute 15-Bit PEC
  uint16_t PEC = calculate_PEC(commmandWord, sizeof(commmandWord));

  // Pack the command + PEC using Big Endian
  uint8_t txBUFFER[4];
  txBUFFER[0] = (uint8_t)((commmandWord >> 8) & 0xFF);
  txBUFFER[1] = (uint8_t)((commmandWord & 0xFF));

  // The PEC Command is a 15-bit represented in 2 bytes as bits [4...7] and
  // [6...0, plus 1 zero bit]. txBUffer[2] = MSB of PEC txBuffer[3] = LSB of PEC
  // (7 lowest bit plus trailing 0) Table 41
  txBUFFER[2] = (uint8_t)((PEC >> 7) & 0xFF);
  txBUFFER[3] = (uint8_t)((PEC & 0x7F) << 1);

  // Send Command
  if (isoSPI_transmit(txBUFFER, 4)) {
    ESP_LOGE(TAG, "Could not Transmit Command");
  }

  // If Theres a Payload for write Commands
  // We send it next
  if (payload && payloadLen > 0) {
    // Append 10-bit PEC to Payload, Table 42
    //
  }
}

/// Poll the Devices for the serial_id and save the value in an array
/// This allows the device to identify the individual modules and use addressed
/// transmission
///
///
void register_devices(uint8_t *ID_response) {
  uint8_t received_pec = extract_pec(ID_response);
}

uint16_t extract_received_pec(uint16_t *rxbuffer) {
  // Pec is split into 2 Locations: Last Word and the extra one bit
  // Given NUM_STACKS * 16 bits = last_word index
  /* uint16_t last_word = data[NUM_STACKS * 16]; */
  /* uint16_t extra_bit = data[(NUM_STACKS * 16) + 1] & 0x0001; */
  /**/
  /* // combine to form the PEC */
  /* return ((last_word << 1) & 0x7FFE) | extra_bit; */
  size_t len = sizeof(rxbuffer) / sizeof(rxbuffer[0]);
  uint16_t CMDandPEC =
      (((uint16_t)rxbuffer[(len - 2) << 8 | rxbuffer[len - 1]]));

  // Command counter (upper 6 bits)
  uint8_t cmdCounter = (CMDandPEC >> 10) & 0x3F;

  // Lower 10 Bits
  uint16_t dataPEC = CMDandPEC & 0x3FF;

  return dataPEC;
}

void parse_voltages(SPI_responses_t *responses) {
  // PEC is located at the end of the response packet
  uint8_t PEC_calculated =
      calculate_PEC(responses->raw, sizeof(responses->raw));
  uint8_t received_pec = extract_pec(responses->values);

  // Check if the PEC is Valid
  if (PEC_calculated != received_pec) {
  }

  size_t active_cell_index = 0;
  // Dependent on number of Stacks available
  //
  // for every stack available
  //
  // Z.B
  // index 0 -9 -> Module 1
  // index 10 - 17 -> Not used
  // index 18 - 27 -> Module 2
  // index 28 - 35 -> Not used
  for (size_t Stack = 0; Stack < NUM_STACKS; Stack++) {

    // increment the index 10 times representing the 10 active cells
    for (size_t Cell = 0; Cell < CELLS_PER_STACK_ACTIVE; Cell++) {
      robin->individual_voltages[Stack][Cell] =
          responses->values[active_cell_index + 1];
    }
    // skip cell 11 to 18 by incrementing with 7.
    active_cell_index = active_cell_index + 7;
  }
}

void parse_Temperature(SPI_responses_t *responses_Temp) {
  // PEC is located at the end of the response packet
  uint8_t PEC_calculated =
      calculate_PEC(responses_Temp->raw, sizeof(responses_Temp->raw));
  uint8_t received_pec = extract_pec(responses_Temp->values);
}
