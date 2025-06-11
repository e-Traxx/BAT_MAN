#ifndef ADBMS6830B_H
#define ADBMS6830B_H

#include <stdint.h>
#include "debug.h"

// ADBMS6830B Commands
#define RDSID_CMD     0x0001  // Read Serial ID command

// PEC15 polynomial
#define PEC15_POLY    0x755B

// Function prototypes
void ADBMS6830B_Init(void);
void ADBMS6830B_WriteCommand(uint16_t cmd);
uint32_t ADBMS6830B_ReadResponse(void);
uint32_t ADBMS6830B_ReadSerialID(void);
uint16_t ADBMS6830B_CalculatePEC(uint8_t *data, uint8_t len);

#endif // ADBMS6830B_H 