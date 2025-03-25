#include "Robin_handler.h"
#include "esp_log.h"
#include "spi_handler.h"
#include "string.h"
#include <stdint.h>

// This is just a collection of bad code used to parse the received data into
// proper structure and load it into the memory

// Parsing Data
// 1. the response is iteratively deserialised and processed.
// 2. the PEC for each data is extracted and checked.
// 3. the data is then placed in its corresponding index in the Robin_container
// template

// The whole structure is just 8 Bits and 8 bits and 8 bits
void parse_voltages(SPI_responses_t *responses) {
  // PEC is located at the end of every 6 registers or otherwise 6 x 8 bits
  // oooooooooor we have 3 x 16 bits in a groups register for Cell voltage
  // values and then 1 x 16 Bit for Data PEC
  size_t cell = 0; // Robin_container
  size_t group = 0;
  uint16_t PEC = 0;
  // for every Module present
  for (size_t stack = 0; stack < NUM_STACKS; stack++) {
    // for every 4 Register Groups (1 - 12)
    while (group < 5 && cell < 10) {
      // Check PEC
      uint8_t Data[3] = {
          responses->values[0],
          responses->values[1],
          responses->values[2],
      };
      if (Compute_Data_PEC(Data, sizeof(Data)) != PEC) {
        // ERROR Message
      }

      robin->individual_voltages[stack][cell] = responses->values[0];

      // if on Group 4, then only take Cell 1 as it is the 10th Cell at
      // index 11.
      if (cell < 8) {
        robin->individual_voltages[stack][cell + 1] = responses->values[1];
        robin->individual_voltages[stack][cell + 2] = responses->values[2];
      }
      PEC = responses->values[3];

      // increment to next Register group
      group += 1;

      // Increment the Cell Counter to access the next cells on the next loop
      cell += 3;
    }
  }
}

/// Is just a Template, so i need to check up on it later.

void parse_Temperature(SPI_responses_t *responses_Temp) {
  // PEC is located at the end of every 6 registers or otherwise 6 x 8 bits
  // oooooooooor we have 3 x 16 bits in a groups register for Cell voltage
  // values and then 1 x 16 Bit for Data PEC
  size_t cell = 0; // Robin_container
  size_t group = 0;
  uint16_t PEC = 0;
  // for every Module present
  for (size_t stack = 0; stack < NUM_STACKS; stack++) {
    // for every 4 Register Groups (1 - 12)
    while (group < 5 && cell < 10) {
      // Check PEC
      uint8_t Data[3] = {
          responses_Temp->values[0],
          responses_Temp->values[1],
          responses_Temp->values[2],
      };
      if (Compute_Data_PEC(Data, sizeof(Data)) != PEC) {
        // ERROR Message
      }

      robin->individual_temperatures[stack][cell] = responses_Temp->values[0];

      // if on Group 4, then only take Cell 1 as it is the 10th Cell at
      // index 11.
      if (cell < 8) {
        robin->individual_temperatures[stack][cell + 1] =
            responses_Temp->values[1];
        robin->individual_temperatures[stack][cell + 2] =
            responses_Temp->values[2];
      }
      PEC = responses_Temp->values[3];

      // increment to next Register group
      group += 1;

      // Increment the Cell Counter to access the next cells on the next loop
      cell += 3;
    }
  }
}

// CAN DATA FORMATTER
//
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
