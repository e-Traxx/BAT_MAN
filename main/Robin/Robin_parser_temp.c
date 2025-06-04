// #include "Robin_handler.h"
// #include "Robin_types.h"
// #include "esp_log.h"
// #include "spi_handler.h"
// #include "string.h"
// #include <stdint.h>
//
// #define TAG "Robin_parser"
//
// #define CELLS_PER_GROUP 3
// // Exp: CVAR0 and CVAR1 are 1 Word
// // We have 3 Cells (6 CV Group) where each cell is a Word of 16-Bit
// // The 3 Words/Cells are followed by 1 PEC (1 x 16-Bit Word)
// // in Total we have 4 Words for a Cell group Register
// #define WORDS_PER_GROUP 4
// #define NUMBER_GROUPS 4 // GROUPS A -> D
// #define MAX_CELLS_PER_STACK 10
// #define PEC_SIZE 2 // PEC is 16 bits
//
// // This is just a collection of bad code used to parse the received data into
// // proper structure and load it into the memory
//
// // Parsing Data
// // 1. the response is iteratively deserialised and processed.
// // 2. the PEC for each data is extracted and checked.
// // 3. the data is then placed in its corresponding index in the
// Robin_container
// // template
//
// // The whole structure is just 8 Bits and 8 bits and 8 bits
// parser_error_t parse_voltages(SPI_responses_t *responses) {
//   if (!responses || !robin) {
//     ESP_LOGE(TAG, "Invalid pointer in parse_voltages");
//     return PARSER_ERROR_NULL_POINTER;
//   }
//
//   // PEC is located at the end of every 6 registers or otherwise 6 x 8 bits
//   // oooooooooor we have 3 x 16 bits in a groups register for Cell voltage
//   // values and then 1 x 16 Bit for Data PEC
//   size_t cell = 0; // Robin_container
//   size_t group = 0;
//   uint16_t PEC = 0;
//   uint8_t Group_offset = 0;
//   // for every Module present
//   for (size_t stack = 0; stack < NUM_STACKS; stack++) {
//     // for every 4 Register Groups (1 - 12)
//     while (group < NUMBER_GROUPS && cell < MAX_CELLS_PER_STACK) {
//       uint16_t Data[CELLS_PER_GROUP] = {
//           responses->values[Group_offset + 0],
//           responses->values[Group_offset + 1],
//           responses->values[Group_offset + 2],
//       };
//       // Check PEC
//       PEC = responses->values[Group_offset + 3];
//
//       if (Compute_Data_PEC(Data, sizeof(Data)) != PEC) {
//         ESP_LOGE(TAG, "PEC mismatch in stack %zu, group %zu", stack, group);
//         return PARSER_ERROR_PEC_MISMATCH;
//       }
//
//       robin->individual_voltages[stack][cell] =
//           responses->values[Group_offset + 0];
//
//       // if on Group 4, then only take Cell 1 as it is the 10th Cell at
//       // index 11.
//       if (cell + 2 < MAX_CELLS_PER_STACK) {
//         robin->individual_voltages[stack][cell + 1] =
//             responses->values[Group_offset + 1];
//         robin->individual_voltages[stack][cell + 2] =
//             responses->values[Group_offset + 2];
//       }
//
//       // increment to next Register group
//       group += 1;
//
//       // Increment the Cell Counter to access the next cells on the next loop
//       cell += CELLS_PER_GROUP;
//     }
//     // Reset counters for next stack
//     cell = 0;
//     group = 0;
//   }
//   return PARSER_OK;
// }
//
// /// Is just a Template, so i need to check up on it later.
// parser_error_t parse_Temperature(SPI_responses_t *responses_Temp) {
//   if (!responses_Temp || !robin) {
//     ESP_LOGE(TAG, "Invalid pointer in parse_Temperature");
//     return PARSER_ERROR_NULL_POINTER;
//   }
//
//   // PEC is located at the end of every 6 registers or otherwise 6 x 8 bits
//   // oooooooooor we have 3 x 16 bits in a groups register for Cell voltage
//   // values and then 1 x 16 Bit for Data PEC
//   size_t cell = 0; // Robin_container
//   size_t group = 0;
//   uint16_t PEC = 0;
//   // for every Module present
//   for (size_t stack = 0; stack < NUM_STACKS; stack++) {
//     // for every 4 Register Groups (1 - 12)
//     while (group < WORDS_PER_GROUP && cell < MAX_CELLS_PER_STACK) {
//       // Check PEC
//       uint8_t Data[CELLS_PER_GROUP] = {
//           responses_Temp->values[0],
//           responses_Temp->values[1],
//           responses_Temp->values[2],
//       };
//       if (Compute_Data_PEC(Data, sizeof(Data)) != PEC) {
//         ESP_LOGE(TAG, "PEC mismatch in stack %zu, group %zu", stack, group);
//         return PARSER_ERROR_PEC_MISMATCH;
//       }
//
//       robin->individual_temperatures[stack][cell] =
//       responses_Temp->values[0];
//
//       // if on Group 4, then only take Cell 1 as it is the 10th Cell at
//       // index 11.
//       if (cell < MAX_CELLS_PER_STACK - 2) {
//         robin->individual_temperatures[stack][cell + 1] =
//             responses_Temp->values[1];
//         robin->individual_temperatures[stack][cell + 2] =
//             responses_Temp->values[2];
//       }
//       PEC = responses_Temp->values[3];
//
//       // increment to next Register group
//       group += 1;
//
//       // Increment the Cell Counter to access the next cells on the next loop
//       cell += CELLS_PER_GROUP;
//     }
//     // Reset counters for next stack
//     cell = 0;
//     group = 0;
//   }
//   return PARSER_OK;
// }
//
// // CAN DATA FORMATTER
// //
// /*
//  *
//  * Parsing Data for and from Communication
//  *
//  */
//
// void individual_voltages_formatter(individual_voltages_frame_t *frame,
//                                    uint16_t voltages[5], uint8_t mux) {
//   if (!frame || !voltages) {
//     ESP_LOGE(TAG, "Invalid pointer in individual_voltages_formatter");
//     return;
//   }
//
//   memset(frame->bytes, 0, sizeof(frame->bytes));
//
//   frame->fields.mux = mux;
//   frame->fields.voltage_1 = voltages[0] & 0x3FF;
//   frame->fields.voltage_2 = voltages[1] & 0x3FF;
//   frame->fields.voltage_3 = voltages[2] & 0x3FF;
//   frame->fields.voltage_4 = voltages[3] & 0x3FF;
//   frame->fields.voltage_5 = voltages[4] & 0x3FF;
//
//   frame->fields.Reserved = 0;
// }
//
// void individual_temperatures_formatter(individual_temperatures_frame_t
// *frame,
//                                        uint16_t temps[5], uint8_t mux) {
//   if (!frame || !temps) {
//     ESP_LOGE(TAG, "Invalid pointer in individual_temperatures_formatter");
//     return;
//   }
//
//   frame->fields.mux = mux;
//   frame->fields.temp_1 = temps[0] & 0x3FF;
//   frame->fields.temp_2 = temps[1] & 0x3FF;
//   frame->fields.temp_3 = temps[2] & 0x3FF;
//   frame->fields.temp_4 = temps[3] & 0x3FF;
//   frame->fields.temp_5 = temps[4] & 0x3FF;
//
//   frame->fields.Reserved = 0;
// }
//
// // Common parsing function for both voltages and temperatures
// static parser_error_t
// parse_cell_data(SPI_responses_t *responses,
//                 uint16_t (*target_array)[MAX_CELLS_PER_STACK],
//                 size_t stack_index) {
//   if (!responses || !target_array) {
//     ESP_LOGE(TAG, "Null pointer in parse_cell_data");
//     return PARSER_ERROR_NULL_POINTER;
//   }
//
//   size_t cell = 0;
//   size_t group = 0;
//   uint16_t PEC = 0;
//
//   while (group < WORDS_PER_GROUP && cell < MAX_CELLS_PER_STACK) {
//     // Check PEC
//     uint8_t Data[CELLS_PER_GROUP] = {responses->values[0],
//     responses->values[1],
//                                      responses->values[2]};
//
//     if (Compute_Data_PEC(Data, sizeof(Data)) != PEC) {
//       ESP_LOGE(TAG, "PEC mismatch in stack %zu, group %zu", stack_index,
//       group); return PARSER_ERROR_PEC_MISMATCH;
//     }
//
//     // Store first cell value
//     target_array[stack_index][cell] = responses->values[0];
//
//     // Store remaining cell values if within bounds
//     if (cell < MAX_CELLS_PER_STACK - 2) {
//       target_array[stack_index][cell + 1] = responses->values[1];
//       target_array[stack_index][cell + 2] = responses->values[2];
//     }
//
//     PEC = responses->values[3];
//     group++;
//     cell += CELLS_PER_GROUP;
//   }
//
//   return PARSER_OK;
// }
