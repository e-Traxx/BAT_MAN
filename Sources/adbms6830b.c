#include "adbms6830b.h"
#include "stm32f1xx_hal.h"
#include "debug.h"
#include <string.h>

// SPI handle declaration
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;  // Virtual COM port on Nucleo

// Chip select pin definition
#define CS_PIN        GPIO_PIN_4
#define CS_PORT       GPIOA

// LED pin definition for Nucleo-F103RB
#define LED_PIN       GPIO_PIN_13
#define LED_PORT      GPIOC

// Debug level
static debug_level_t current_debug_level = DEBUG_INFO;

void ADBMS6830B_SetDebugLevel(debug_level_t level)
{
    current_debug_level = level;
}

void ADBMS6830B_DebugPrint(debug_level_t level, const char *format, ...)
{
    if (level > current_debug_level) return;
    
    char buffer[256];
    va_list args;
    
    // Add debug level prefix
    const char *level_str;
    switch(level) {
        case DEBUG_ERROR:  level_str = "[ERROR] "; break;
        case DEBUG_WARN:   level_str = "[WARN]  "; break;
        case DEBUG_INFO:   level_str = "[INFO]  "; break;
        case DEBUG_VERBOSE: level_str = "[VERB]  "; break;
        default:           level_str = ""; break;
    }
    
    // Format the message
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    // Send via UART
    char final_buffer[300];
    snprintf(final_buffer, sizeof(final_buffer), "%s%s\r\n", level_str, buffer);
    HAL_UART_Transmit(&huart2, (uint8_t*)final_buffer, strlen(final_buffer), 100);
}

void ADBMS6830B_Init(void)
{
    // Configure SPI for ADBMS6830B
    // Mode 3: CPOL=1, CPHA=1 for isoSPI
    // MSB first
    // 16-bit data frame
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;  // CPOL=1
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;      // CPHA=1
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;  // For 1MHz with 8MHz APB2 clock
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    
    HAL_SPI_Init(&hspi1);
    
    // Initialize LED pin
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &GPIO_InitStruct);
    
    DEBUG_INFO("ADBMS6830B initialized");
}

void ADBMS6830B_WriteCommand(uint16_t cmd)
{
    // Toggle LED to indicate transaction start
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
    
    // Calculate PEC for command
    uint8_t cmd_bytes[2] = {(uint8_t)(cmd >> 8), (uint8_t)cmd};
    uint16_t pec = ADBMS6830B_CalculatePEC(cmd_bytes, 2);
    
    // Pull CS low to start transaction
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    
    // Send command
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&cmd, 1, 100);
    
    // Send PEC
    HAL_SPI_Transmit(&hspi1, (uint8_t*)&pec, 1, 100);
    
    // Pull CS high to end transaction
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
    
    DEBUG_VERBOSE("Sent command: 0x%04X, PEC: 0x%04X", cmd, pec);
}

uint32_t ADBMS6830B_ReadResponse(void)
{
    uint32_t response = 0;
    uint16_t rx_data;
    uint16_t received_pec;
    uint8_t response_bytes[6];
    
    // Pull CS low to start transaction
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);
    
    // Read response (3 words for RDSID)
    for(int i = 0; i < 3; i++) {
        HAL_SPI_Receive(&hspi1, (uint8_t*)&rx_data, 1, 100);
        response = (response << 16) | rx_data;
        response_bytes[i*2] = (uint8_t)(rx_data >> 8);
        response_bytes[i*2+1] = (uint8_t)rx_data;
    }
    
    // Read PEC
    HAL_SPI_Receive(&hspi1, (uint8_t*)&received_pec, 1, 100);
    
    // Calculate PEC for received data
    uint16_t calculated_pec = ADBMS6830B_CalculatePEC(response_bytes, 6);
    
    // Pull CS high to end transaction
    HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);
    
    // Toggle LED to indicate transaction end
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
    
    // Verify PEC
    if(calculated_pec != received_pec) {
        DEBUG_ERROR("PEC Error! Received: 0x%04X, Calculated: 0x%04X", 
                   received_pec, calculated_pec);
        return 0xFFFFFFFF; // Error value
    }
    
    DEBUG_INFO("Received ID: 0x%012lX", response);
    return response;
}

uint32_t ADBMS6830B_ReadSerialID(void)
{
    DEBUG_INFO("Reading device ID...");
    // Send RDSID command
    ADBMS6830B_WriteCommand(RDSID_CMD);
    
    // Read response
    return ADBMS6830B_ReadResponse();
}

uint16_t ADBMS6830B_CalculatePEC(uint8_t *data, uint8_t len)
{
    uint16_t remainder = 0x0000;
    uint8_t i, j;
    
    for(i = 0; i < len; i++)
    {
        remainder ^= (uint16_t)data[i] << 7;
        for(j = 0; j < 8; j++)
        {
            if(remainder & 0x8000)
            {
                remainder = (remainder << 1) ^ PEC15_POLY;
            }
            else
            {
                remainder = remainder << 1;
            }
        }
    }
    return remainder;
} 