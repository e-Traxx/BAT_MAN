#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include <stdarg.h>

// Debug levels
typedef enum {
    DEBUG_NONE = 0,
    DEBUG_ERROR,
    DEBUG_WARN,
    DEBUG_INFO,
    DEBUG_VERBOSE
} debug_level_t;

// Debug output interfaces
typedef enum {
    DEBUG_OUTPUT_NONE = 0,
    DEBUG_OUTPUT_UART,
    DEBUG_OUTPUT_SWO,
    DEBUG_OUTPUT_BOTH
} debug_output_t;

// Debug configuration structure
typedef struct {
    debug_level_t level;
    debug_output_t output;
    uint32_t uart_baudrate;
    uint8_t enable_timestamp;
} debug_config_t;

// Function prototypes
void DEBUG_Init(debug_config_t *config);
void DEBUG_SetLevel(debug_level_t level);
void DEBUG_SetOutput(debug_output_t output);
void DEBUG_Print(debug_level_t level, const char *format, ...);
void DEBUG_PrintRaw(const char *format, ...);
void DEBUG_EnableTimestamp(uint8_t enable);

// Convenience macros
#define DEBUG_ERROR(fmt, ...)   DEBUG_Print(DEBUG_ERROR, fmt, ##__VA_ARGS__)
#define DEBUG_WARN(fmt, ...)    DEBUG_Print(DEBUG_WARN, fmt, ##__VA_ARGS__)
#define DEBUG_INFO(fmt, ...)    DEBUG_Print(DEBUG_INFO, fmt, ##__VA_ARGS__)
#define DEBUG_VERBOSE(fmt, ...) DEBUG_Print(DEBUG_VERBOSE, fmt, ##__VA_ARGS__)

#endif /* DEBUG_H */ 