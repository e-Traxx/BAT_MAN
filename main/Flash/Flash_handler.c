#include "Flash_handler.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "Flash Handler";

// Global flash handler instance
flash_handler_t flash_handler = {
    .state = FLASH_STATE_UNINITIALIZED,
    .config_loaded = false
};

// Default configuration values
static const system_config_t default_config = {
    // Battery limits
    .overvoltage_limit = 4150,    // 4.15V
    .undervoltage_limit = 3000,   // 3.00V
    .high_temp_limit = 550,       // 55.0°C
    .low_temp_limit = 100,        // 10.0°C
    
    // Cell balancing parameters
    .balancing_threshold = 20,    // 20mV difference to trigger balancing
    .balancing_duration = 1000,   // 1 second balancing duration
    .max_balancing_cells = 3,     // Maximum 3 cells balancing simultaneously
    
    // System configuration
    .num_stacks = 14,             // Number of battery stacks
    .cells_per_stack = 18,        // Total cells per stack (including dummy)
    .active_cells_per_stack = 10, // Active cells per stack (excluding dummy)
    
    // Communication settings
    .can_baud_rate = 1000000,     // 1Mbps
    .wifi_channel = 1,            // WiFi channel 1
    .wifi_max_connections = 5,    // Maximum 5 WiFi connections
    .can_node_id = 0x100,         // Default CAN node ID
    
    // Calibration data
    .current_sensor_offset = 0,   // No offset
    .voltage_calibration = 0,     // No voltage calibration
    .temp_calibration = 0,        // No temperature calibration
    .current_gain = 1000,         // Default gain factor (1000 = 1.000)
    
    // Battery characteristics
    .nominal_capacity = 50000,    // 50Ah
    .max_charge_current = 50000,  // 50A max charge current
    .max_discharge_current = 100000, // 100A max discharge current
    .nominal_voltage = 36000,     // 36V nominal voltage
    
    // Safety parameters
    .overcurrent_limit = 120000,  // 120A overcurrent limit
    .short_circuit_limit = 200000,// 200A short circuit limit
    .overtemp_charge_limit = 450, // 45°C max charge temperature
    .overtemp_discharge_limit = 550, // 55°C max discharge temperature
    
    // Timing parameters
    .measurement_interval = 200,  // 200ms between measurements
    .balancing_interval = 1000,   // 1s between balancing checks
    .diagnostic_interval = 1000,  // 1s between diagnostic checks
    
    // Reserved space
    .reserved = {0}
};

esp_err_t Flash_Setup(void) {
    esp_err_t ret;

    // Initialize flash storage
    ret = flash_storage_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize flash storage");
        flash_handler.state = FLASH_STATE_ERROR;
        return ret;
    }

    // Try to load existing configuration
    ret = Flash_Load_Config();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "No valid configuration found, using defaults");
        memcpy(&flash_handler.config, &default_config, sizeof(system_config_t));
        flash_handler.config_loaded = true;
    }

    flash_handler.state = FLASH_STATE_INITIALIZED;
    ESP_LOGI(TAG, "Flash handler initialized");
    return ESP_OK;
}

esp_err_t Flash_Load_Config(void) {
    if (flash_handler.state != FLASH_STATE_INITIALIZED) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = flash_storage_read_config(&flash_handler.config);
    if (ret == ESP_OK) {
        flash_handler.config_loaded = true;
        ESP_LOGI(TAG, "Configuration loaded successfully");
    } else {
        ESP_LOGE(TAG, "Failed to load configuration");
    }

    return ret;
}

esp_err_t Flash_Save_Config(void) {
    if (flash_handler.state != FLASH_STATE_INITIALIZED) {
        return ESP_ERR_INVALID_STATE;
    }

    esp_err_t ret = flash_storage_write_config(&flash_handler.config);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Configuration saved successfully");
    } else {
        ESP_LOGE(TAG, "Failed to save configuration");
    }

    return ret;
}

esp_err_t Flash_Reset_Config(void) {
    if (flash_handler.state != FLASH_STATE_INITIALIZED) {
        return ESP_ERR_INVALID_STATE;
    }

    // Erase current configuration
    esp_err_t ret = flash_storage_erase_config();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to erase configuration");
        return ret;
    }

    // Load default configuration
    memcpy(&flash_handler.config, &default_config, sizeof(system_config_t));
    flash_handler.config_loaded = true;

    // Save default configuration
    ret = Flash_Save_Config();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to save default configuration");
        return ret;
    }

    ESP_LOGI(TAG, "Configuration reset to defaults");
    return ESP_OK;
}

bool Flash_Is_Config_Loaded(void) {
    return flash_handler.config_loaded;
}

system_config_t* Flash_Get_Config(void) {
    if (!flash_handler.config_loaded) {
        return NULL;
    }
    return &flash_handler.config;
} 