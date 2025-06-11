#include "debug.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

// External handles
extern UART_HandleTypeDef huart2;

// Static variables
static debug_config_t debug_config = {
    .level = DEBUG_INFO,
    .output = DEBUG_OUTPUT_UART,
    .uart_baudrate = 115200,
    .enable_timestamp = 1
};

// Get timestamp in milliseconds
static uint32_t get_timestamp(void)
{
    return HAL_GetTick();
}

// Get debug level string
static const char* get_level_string(debug_level_t level)
{
    switch(level) {
        case DEBUG_ERROR:  return "[ERROR] ";
        case DEBUG_WARN:   return "[WARN]  ";
        case DEBUG_INFO:   return "[INFO]  ";
        case DEBUG_VERBOSE: return "[VERB]  ";
        default:           return "";
    }
}

void DEBUG_Init(debug_config_t *config)
{
    if (config != NULL) {
        memcpy(&debug_config, config, sizeof(debug_config_t));
    }
    
    // Initialize UART if needed
    if (debug_config.output & DEBUG_OUTPUT_UART) {
        huart2.Instance = USART2;
        huart2.Init.BaudRate = debug_config.uart_baudrate;
        huart2.Init.WordLength = UART_WORDLENGTH_8B;
        huart2.Init.StopBits = UART_STOPBITS_1;
        huart2.Init.Parity = UART_PARITY_NONE;
        huart2.Init.Mode = UART_MODE_TX_RX;
        huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
        huart2.Init.OverSampling = UART_OVERSAMPLING_16;
        
        if (HAL_UART_Init(&huart2) != HAL_OK) {
            // Error handling
        }
    }
    
    DEBUG_INFO("Debug system initialized");
}

void DEBUG_SetLevel(debug_level_t level)
{
    debug_config.level = level;
}

void DEBUG_SetOutput(debug_output_t output)
{
    debug_config.output = output;
}

void DEBUG_EnableTimestamp(uint8_t enable)
{
    debug_config.enable_timestamp = enable;
}

void DEBUG_Print(debug_level_t level, const char *format, ...)
{
    if (level > debug_config.level) return;
    
    char buffer[256];
    va_list args;
    
    // Format the message
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Prepare final message with timestamp and level
    char final_buffer[300];
    if (debug_config.enable_timestamp) {
        snprintf(final_buffer, sizeof(final_buffer), "[%lu] %s%s\r\n", 
                get_timestamp(), get_level_string(level), buffer);
    } else {
        snprintf(final_buffer, sizeof(final_buffer), "%s%s\r\n", 
                get_level_string(level), buffer);
    }
    
    // Send via configured outputs
    if (debug_config.output & DEBUG_OUTPUT_UART) {
        HAL_UART_Transmit(&huart2, (uint8_t*)final_buffer, strlen(final_buffer), 100);
    }
    
    if (debug_config.output & DEBUG_OUTPUT_SWO) {
        // SWO implementation would go here
        // ITM_SendChar() for each character
    }
}

void DEBUG_PrintRaw(const char *format, ...)
{
    char buffer[256];
    va_list args;
    
    // Format the message
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Send via configured outputs
    if (debug_config.output & DEBUG_OUTPUT_UART) {
        HAL_UART_Transmit(&huart2, (uint8_t*)buffer, strlen(buffer), 100);
    }
    
    if (debug_config.output & DEBUG_OUTPUT_SWO) {
        // SWO implementation would go here
    }
} 