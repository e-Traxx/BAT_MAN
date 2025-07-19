#ifndef ROBINH_H
#define ROBINH_H

#include "freertos/idf_additions.h"
#include "stdint.h"
#include <stdint.h>

#define NUM_STACKS 1 // Normally 14
#define CELLS_PER_STACK_ACTIVE 10
#define CELLS_PER_STACK 18
#define RESPONSE_SIZE 13 // BYTES
#define TOTAL_CELLS (NUM_STACKS * CELLS_PER_STACK)

// ADBMS6830 Frame Geometry
#define REG_GROUPS 6 // A - F
#define MAX_CELLS 10 // Number of active Cells
#define MAX_AUX_WORDS (REG_GROUPS * 3)
#define MODULE_COUNT 14 // Devices in Daisy Chain

// Data Geometry
#define DATA_BYTES_PER_GROUP 6 // 3 x 16-Bit Words
#define BYTES_PER_GROUP 8      // 6 x 8-Bit + 2 PEC
#define BYTES_PER_DEVICE (REG_GROUPS * BYTES_PER_GROUP)
#define PAYLOAD_LENGTH (NUM_STACKS * BYTES_PER_DEVICE)

void ADBMS_Setup (void);
void Adbms_query_callback (TimerHandle_t timer);
void Robin_query (void *args);
uint8_t calculate_PEC (const uint8_t *data, size_t len);

typedef struct
{
    uint16_t individual_voltages[14][10];
    uint16_t individual_temperatures[14][10];
    uint16_t overall_voltage;
    uint16_t overall_temperature;
    uint16_t overall_current;
    uint16_t overall_SOC;
} Robin_container_t;

// CAN Message Template
typedef struct
{
    uint32_t mux : 8;
    uint32_t voltage_1 : 10;
    uint32_t voltage_2 : 10;
    uint32_t voltage_3 : 10;
    uint32_t voltage_4 : 10;
    uint32_t voltage_5 : 10;

    uint32_t Reserved : 6;

} __attribute__ ((packed)) individual_voltages_message_t;

typedef struct
{
    uint32_t mux : 8;
    uint32_t temp_1 : 10;
    uint32_t temp_2 : 10;
    uint32_t temp_3 : 10;
    uint32_t temp_4 : 10;
    uint32_t temp_5 : 10;

    uint32_t Reserved : 6;

} __attribute__ ((packed)) individual_temperatures_message_t;

// The physical frame in bytes
typedef union
{
    individual_voltages_message_t fields;
    uint8_t bytes[8];
} individual_voltages_frame_t;

typedef union
{
    individual_temperatures_message_t fields;
    uint8_t bytes[8];
} individual_temperatures_frame_t;

// Bit masking Parsers
void individual_voltages_formatter (individual_voltages_frame_t *frame, uint16_t voltages[5], uint8_t mux);
void individual_temperatures_formatter (individual_temperatures_frame_t *frame, uint16_t temps[5], uint8_t mux);

// SPI Transmission
//
// Each stack has 18 Cells (16 Cells + 2 Dummy)
// Each module has 6 Cell voltage register Group
// Each Cell voltage register Group has 6 registers. (3 Cells)
// Each Register has its own 10 bit PEC.
// 1 Module = (6 registers x 1 Byte) + 2 Byte PEC
// 1 Module = 8 Bytes x 6 register Groups
// 1 Module = 48 Bytes
//
// 14 modules = 48 Bytes x 14 modules
// 14 Modules = 672 Bytes

// 14 Modules * (8 Bytes * (6 registers + 2 PEC) * 6 groups) * 8 bits

typedef union
{
    uint8_t raw[PAYLOAD_LENGTH];
    // 16 bit saved in 2 locations sequentially as 8 bits, therefore:
    // PAYLOAD_LENGTH * 2
    uint16_t Words[PAYLOAD_LENGTH / 2];
} SPI_responses_t;

// Serial ID
extern uint8_t ADBMS_ID[(6 * NUM_STACKS) + 2];

// handlers
extern Robin_container_t *robin;
extern SemaphoreHandle_t adbms_semaphore;
extern TimerHandle_t adbms_timer;

// Parsers
int copy_group_word (const uint8_t *grp_bytes, uint16_t *dest, int destidx, int dest_max);
int parse_rdcvall (const uint8_t *rx, uint16_t *dest);
int parse_rdauxall (const uint8_t *rx, uint16_t *dest);
bool parse_voltages (const uint8_t *chain_rawdata);
bool parse_temperatures (const uint8_t *chain_rawdata);

#endif
