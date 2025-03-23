#include "Robin_handler.h"
#include "string.h"

// This is just a collection of bad code used to parse the received data into
// proper structure and load it into the memory

// Parsing Data
// 1. the response is iteratively deserialised and processed.
// 2. the PEC for each data is extracted and checked.
// 3. the data is then placed in its corresponding index in the Robin_container
// template

// The whole structure is just 8 Bits and 8 bits and 8 bits
void parse_voltages(SPI_responses_t *responses) {
  /* // PEC is located at the end of the response packet */
  /* uint8_t PEC_calculated = */
  /*     calculate_PEC(responses->raw, sizeof(responses->raw)); */
  /* uint8_t received_pec = extract_pec(responses->values); */
  /**/
  /* // Check if the PEC is Valid */
  /* if (PEC_calculated != received_pec) { */
  /* } */
  /**/
  /* size_t active_cell_index = 0; */
  /* // Dependent on number of Stacks available */
  /* // */
  /* // for every stack available */
  /* // */
  /* // Z.B */
  /* // index 0 -9 -> Module 1 */
  /* // index 10 - 17 -> Not used */
  /* // index 18 - 27 -> Module 2 */
  /* // index 28 - 35 -> Not used */
  /* for (size_t Stack = 0; Stack < NUM_STACKS; Stack++) { */
  /**/
  /*   // increment the index 10 times representing the 10 active cells */
  /*   for (size_t Cell = 0; Cell < CELLS_PER_STACK_ACTIVE; Cell++) { */
  /*     robin->individual_voltages[Stack][Cell] = */
  /*         responses->values[active_cell_index + 1]; */
  /*   } */
  /*   // skip cell 11 to 18 by incrementing with 7. */
  /*   active_cell_index = active_cell_index + 7; */
  /* } */

  // PEC is located at the end of every 6 registers or otherwise 6 x 8 bits
  // oooooooooor we have 3 x 16 bits in a groups register for Cell voltage
  // values and then 1 x 16 Bit for Data PEC
  size_t index = 0;
  size_t cell = 0; // Robin_container
                   // for every Module present
  for (size_t stack = 0; stack < NUM_STACKS; stack++) {
    // for every 4 Register Groups (1 - 12)
    for (size_t group = 0; group < 5; group++) {
      // Every 4 x 16-bits
      if (cell < 10) {
        robin->individual_voltages[cell][stack] = responses->values[0];
        robin->individual_voltages[cell + 1][stack] = responses->values[2];
        robin->individual_voltages[cell + 2][stack] = responses->values[3];
      } else {
        robin->individual_voltages[cell][stack] = responses->values[0];
      }
      cell += 3;
    }

    cell = 0;
  }
}

void parse_Temperature(SPI_responses_t *responses_Temp) {
  // PEC is located at the end of the response packet
  uint8_t PEC_calculated =
      calculate_PEC(responses_Temp->raw, sizeof(responses_Temp->raw));
  uint8_t received_pec = extract_pec(responses_Temp->values);
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
