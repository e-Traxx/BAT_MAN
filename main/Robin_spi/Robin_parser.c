#include "Batman_esp.h"
#include "Robin_handler.h"
#include "Robin_types.h"
#include "esp_log.h"
#include "spi_handler.h"
#include "string.h"
#include <stdint.h>

#define TAG "Robin_parser"

typedef struct __attribute__ ((packed))
{
    uint8_t data[DATA_BYTES_PER_GROUP];
    uint16_t PEC_be;
} reg_group_t;

extern uint16_t Compute_Data_PEC (const uint8_t *data, size_t data_length);

//
//  Overall Goal:
//
//  Take values from the raw data and place it into global robin
//  which has arrays of size [14][10]
//  for 14 Modules and 10 Cells
//
//  example: robin->individual_temperatures is an [14][10] array
//
//
//
//
int
copy_group_word (const uint8_t *grp_bytes, uint16_t *dest, int destidx, int dest_max)
{
    if (!grp_bytes || !dest)
	{
	    ESP_LOGE (TAG, "Invalid pointer in parse_voltages");
	    return -1;
	}

    // Verify PEC

    // PEC is located at the end of every 6 registers or otherwise 6 x 8 bits
    // oooooooooor we have 3 x 16 bits in a groups register for Cell voltage
    // values and then 1 x 16 Bit for Data PEC

    const reg_group_t *grp = (const reg_group_t *)grp_bytes;

    // CRC 15 Check (big endian -> host order)
    uint16_t rx_pec = (grp->PEC_be >> 8) | (grp->PEC_be << 8);
    if (Compute_Data_PEC (grp->data, DATA_BYTES_PER_GROUP) != rx_pec)
	{
	    return -1;
	}

    // Store 3 Words = 3 x 16-Bit Words
    for (int w = 0; w < 3 && destidx < dest_max; ++w, ++destidx)
	{
	    dest[destidx] = (grp->data[2 * w] << 8) | grp->data[2 * w + 1];
	}

    return destidx;
}

// Parses the Cell voltage values
//
// rx :  Raw data received from the ISOSPI
// dest : 1D array from robin->individual_voltages[Module][10]
int
parse_rdcvall (const uint8_t *rx, uint16_t *dest)
{
    int idx = 0;

    //
    for (int g = 0; g < REG_GROUPS && idx >= 0 && idx < MAX_CELLS; g++)
	{
	    idx = copy_group_word (&rx[g * BYTES_PER_GROUP], dest, idx, MAX_CELLS);
	    if (idx < 0)
		return -1;
	}
    return (idx < 0) ? -1 : 0;
}

// Parses Temperature Values
//
// rx :  Raw data received from the ISOSPI
// dest : 1D array from robin->individual_voltages[Module][10]
int
parse_rdauxall (const uint8_t *rx, uint16_t *dest)
{
    int idx = 0;
    for (int g = 0; g < REG_GROUPS && idx >= 0 && idx < MAX_AUX_WORDS; ++g)
	{
	    idx = copy_group_word (&rx[g * BYTES_PER_GROUP], dest, idx, MAX_AUX_WORDS);
	}
    return (idx < 0) ? -1 : 0;
}

// Public API
//
// Using offset m * BYTES_PER_DEVICE
// where 48 Bytes are skipped to reach the next module
//
//
bool
parse_voltages (const uint8_t *chain_rawdata)
{

    size_t chainbytes = MODULE_COUNT * BYTES_PER_DEVICE;
    if (!chain_rawdata)
	return false;

    // for each Module
    for (int m = 0; m < MODULE_COUNT; ++m)
	{
	    // create a Frame using offset
	    const uint8_t *frame = &chain_rawdata[m * BYTES_PER_DEVICE];
	    if (parse_rdcvall (frame, robin->individual_voltages[m]) < 0)
		{
		    return false;
		}
	}

    return true;
}

bool
parse_temperatures (const uint8_t *chain_rawdata)
{

    size_t chainbytes = MODULE_COUNT * BYTES_PER_DEVICE;
    if (!chain_rawdata)
	return false;

    // for each Module
    for (int m = 0; m < MODULE_COUNT; ++m)
	{
	    // create a Frame using offset
	    const uint8_t *frame = &chain_rawdata[m * BYTES_PER_DEVICE];
	    if (parse_rdcvall (frame, robin->individual_voltages[m]) < 0)
		{
		    return false;
		}
	}

    return true;
}

// CAN DATA FORMATTER
//
/*
 *
 * Parsing Data for and from Communication
 *
 */

void
individual_voltages_formatter (individual_voltages_frame_t *frame, uint16_t voltages[5], uint8_t mux)
{
    if (!frame || !voltages)
	{
	    ESP_LOGE (TAG, "Invalid pointer in individual_voltages_formatter");
	    return;
	}

    memset (frame->bytes, 0, sizeof (frame->bytes));

    frame->fields.mux = mux;
    frame->fields.voltage_1 = voltages[0] & 0x3FF;
    frame->fields.voltage_2 = voltages[1] & 0x3FF;
    frame->fields.voltage_3 = voltages[2] & 0x3FF;
    frame->fields.voltage_4 = voltages[3] & 0x3FF;
    frame->fields.voltage_5 = voltages[4] & 0x3FF;

    frame->fields.Reserved = 0;
}

void
individual_temperatures_formatter (individual_temperatures_frame_t *frame, uint16_t temps[5], uint8_t mux)
{
    if (!frame || !temps)
	{
	    ESP_LOGE (TAG, "Invalid pointer in individual_temperatures_formatter");
	    return;
	}

    frame->fields.mux = mux;
    frame->fields.temp_1 = temps[0] & 0x3FF;
    frame->fields.temp_2 = temps[1] & 0x3FF;
    frame->fields.temp_3 = temps[2] & 0x3FF;
    frame->fields.temp_4 = temps[3] & 0x3FF;
    frame->fields.temp_5 = temps[4] & 0x3FF;

    frame->fields.Reserved = 0;
}
