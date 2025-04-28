#ifndef FLASH_HANDLER_H
#define FLASH_HANDLER_H

#include "flash_storage.h"
#include "esp_err.h"
#include <stdbool.h>

// Flash handler states
typedef enum {
    FLASH_STATE_UNINITIALIZED = 0,
    FLASH_STATE_INITIALIZED,
    FLASH_STATE_ERROR
} flash_state_t;

// Flash handler structure
typedef struct {
    flash_state_t state;
    system_config_t config;
    bool config_loaded;
} flash_handler_t;

// Function declarations
esp_err_t Flash_Setup(void);
esp_err_t Flash_Load_Config(void);
esp_err_t Flash_Save_Config(void);
esp_err_t Flash_Reset_Config(void);
bool Flash_Is_Config_Loaded(void);
system_config_t* Flash_Get_Config(void);

// Global flash handler instance
extern flash_handler_t flash_handler;

#endif // FLASH_HANDLER_H 
