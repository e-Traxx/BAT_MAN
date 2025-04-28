#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

// Flash memory configuration
#define FLASH_SPI_HOST    SPI3_HOST
#define FLASH_CS_PIN      15  // Customize based on your hardware setup
#define FLASH_CLOCK_SPEED 1000000  // 1MHz

// Cell statistics structure
typedef struct {
    // System-wide extremes
    struct {
        uint16_t max_voltage;      // Highest voltage in any cell
        uint16_t min_voltage;      // Lowest voltage in any cell
        uint16_t max_temp;         // Highest temperature in any cell
        uint16_t min_temp;         // Lowest temperature in any cell
        int16_t max_current;       // Highest current drawn (positive for discharge)
        int16_t min_current;       // Lowest current (negative for charge)
        uint32_t timestamp;        // When this was recorded
        uint8_t stack_num;         // Stack number where extreme occurred
        uint8_t cell_num;          // Cell number where extreme occurred
    } extremes;
} system_stats_t;

// System variables structure
typedef struct {
    // Battery limits
    uint16_t overvoltage_limit;    // mV
    uint16_t undervoltage_limit;   // mV
    uint16_t high_temp_limit;      // °C * 10
    uint16_t low_temp_limit;       // °C * 10
    
    // Cell balancing parameters
    uint16_t balancing_threshold;  // mV difference to trigger balancing
    uint16_t balancing_duration;   // ms, how long to balance for
    uint8_t max_balancing_cells;   // Maximum number of cells to balance simultaneously
    
    // System configuration
    uint8_t num_stacks;            // Number of battery stacks
    uint8_t cells_per_stack;       // Number of cells per stack
    uint8_t active_cells_per_stack;// Number of active cells per stack (excluding dummy cells)
    
    // Communication settings
    uint32_t can_baud_rate;        // CAN bus baud rate
    uint8_t wifi_channel;          // WiFi channel
    uint8_t wifi_max_connections;  // Maximum WiFi connections
    uint32_t can_node_id;          // CAN node ID for this BMS
    
    // Calibration data
    int16_t current_sensor_offset; // Current sensor offset
    int16_t voltage_calibration;   // Voltage calibration factor
    int16_t temp_calibration;      // Temperature sensor calibration
    int16_t current_gain;          // Current sensor gain factor
    
    // Battery characteristics
    uint16_t nominal_capacity;     // mAh
    uint16_t max_charge_current;   // mA
    uint16_t max_discharge_current;// mA
    uint16_t nominal_voltage;      // mV
    
    // Safety parameters
    uint16_t overcurrent_limit;    // mA
    uint16_t short_circuit_limit;  // mA
    uint16_t overtemp_charge_limit;// °C * 10
    uint16_t overtemp_discharge_limit; // °C * 10
    
    // Timing parameters
    uint16_t measurement_interval; // ms between measurements
    uint16_t balancing_interval;   // ms between balancing checks
    uint16_t diagnostic_interval;  // ms between diagnostic checks

    // System statistics
    system_stats_t system_stats;   // Track system-wide extremes
    
    // Reserved for future use
    uint8_t reserved[16];
} system_config_t;

// Function declarations
esp_err_t flash_storage_init(void);
esp_err_t flash_storage_read_config(system_config_t *config);
esp_err_t flash_storage_write_config(const system_config_t *config);
esp_err_t flash_storage_erase_config(void);
bool flash_storage_is_initialized(void);

// Cell statistics functions
esp_err_t flash_storage_update_cell_stats(uint8_t stack_num, uint8_t cell_num, 
                                        uint16_t voltage, uint16_t temperature, 
                                        uint32_t timestamp);
esp_err_t flash_storage_get_cell_stats(system_stats_t *stats);
esp_err_t flash_storage_reset_cell_stats(void);

#endif // FLASH_STORAGE_H 