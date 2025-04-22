#include "driver/spi_slave.h"
#include "esp_log.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"

#define SPI_HOST SPI3_HOST
// ESP32-S3 GPIO pins for SPI
#define SPI_MOSI 11
#define SPI_MISO 13
#define SPI_CLK 12
#define SPI_CS 10

// ADBMS6830 Constants
#define TOTAL_MODULES 14
#define CELLS_PER_DEVICE 16
#define GPIO_PINS 10
#define CFG_REG_SIZE 6
#define STAT_REG_SIZE 6
#define AUX_REG_SIZE 6

// Command definitions
#define RDCVA  0x04  // Read Cell Voltages A
#define RDCVB  0x06  // Read Cell Voltages B
#define RDCVC  0x08  // Read Cell Voltages C
#define RDAUXA 0x0C  // Read Auxiliary Register A
#define RDAUXB 0x0E  // Read Auxiliary Register B
#define RDSTATA 0x10 // Read Status Register A
#define RDSTATB 0x12 // Read Status Register B
#define WRCFGA 0x01  // Write Configuration Register A
#define WRCFGB 0x24  // Write Configuration Register B
#define RDCFGA 0x02  // Read Configuration Register A
#define RDCFGB 0x26  // Read Configuration Register B
#define PLADC  0x14  // Poll ADC Conversion Status
#define DIAGN  0x15  // Diagnostic Command

static const char *TAG = "ADBMS6830_SIM";

// ADBMS6830 Register Structure
typedef struct {
    uint16_t cell_voltages[CELLS_PER_DEVICE];  // Cell voltage measurements
    uint16_t gpio_voltages[GPIO_PINS];         // GPIO voltage measurements
    uint8_t config_reg[CFG_REG_SIZE];          // Configuration register
    uint8_t status_reg[STAT_REG_SIZE];         // Status register
    uint8_t aux_reg[AUX_REG_SIZE];             // Auxiliary register
} ADBMS6830_Registers_t;

// Simulated device data
ADBMS6830_Registers_t device_registers[TOTAL_MODULES];
volatile bool conversion_complete = false;
volatile bool adc_busy = false;

// Generate random float in range
float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

// Calculate PEC (Packet Error Code)
uint8_t calculate_pec(uint8_t *data, size_t len) {
    uint8_t pec = 0;
    for (size_t i = 0; i < len; i++) {
        pec ^= data[i];
    }
    return pec;
}

// Initialize device registers
void init_device_registers() {
    for (int i = 0; i < TOTAL_MODULES; i++) {
        // Initialize cell voltages (3.00V - 4.12V)
        for (int j = 0; j < CELLS_PER_DEVICE; j++) {
            float voltage = random_float(3.00, 4.12);
            device_registers[i].cell_voltages[j] = (uint16_t)(voltage * 1000);
        }
        
        // Initialize GPIO voltages (0.5V - 3.6V)
        for (int j = 0; j < GPIO_PINS; j++) {
            float voltage = random_float(0.5, 3.6);
            device_registers[i].gpio_voltages[j] = (uint16_t)(voltage * 1000);
        }
        
        // Initialize configuration register
        device_registers[i].config_reg[0] = 0x00; // Default configuration
        device_registers[i].config_reg[1] = 0x00;
        device_registers[i].config_reg[2] = 0x00;
        device_registers[i].config_reg[3] = 0x00;
        device_registers[i].config_reg[4] = 0x00;
        device_registers[i].config_reg[5] = 0x00;
        
        // Initialize status register
        device_registers[i].status_reg[0] = 0x00; // Default status
        device_registers[i].status_reg[1] = 0x00;
        device_registers[i].status_reg[2] = 0x00;
        device_registers[i].status_reg[3] = 0x00;
        device_registers[i].status_reg[4] = 0x00;
        device_registers[i].status_reg[5] = 0x00;
        
        // Initialize auxiliary register
        device_registers[i].aux_reg[0] = 0x00; // Default auxiliary
        device_registers[i].aux_reg[1] = 0x00;
        device_registers[i].aux_reg[2] = 0x00;
        device_registers[i].aux_reg[3] = 0x00;
        device_registers[i].aux_reg[4] = 0x00;
        device_registers[i].aux_reg[5] = 0x00;
    }
}

// Simulate ADC conversion
void start_adc_conversion() {
    adc_busy = true;
    conversion_complete = false;
    
    // Simulate different conversion times based on configuration
    uint32_t conversion_time = 2; // Default 2ms
    
    // Check if filtering is enabled in config register
    if (device_registers[0].config_reg[0] & 0x01) {
        conversion_time += 2; // Add 2ms for filtering
    }
    
    vTaskDelay(pdMS_TO_TICKS(conversion_time));
    conversion_complete = true;
    adc_busy = false;
}

// Handle incoming SPI commands
void handle_spi_command(uint8_t *cmd, uint8_t *response, size_t length) {
    uint8_t command = cmd[0];
    uint8_t pec = calculate_pec(cmd, 1);
    
    // Verify PEC
    if (cmd[1] != pec) {
        ESP_LOGE(TAG, "PEC Error in command");
        memset(response, 0xFF, length);
        return;
    }
    
    switch (command) {
        case RDCVA:
        case RDCVB:
        case RDCVC:
            ESP_LOGI(TAG, "Read Cell Voltages Command: 0x%02X", command);
            if (!conversion_complete) {
                ESP_LOGW(TAG, "Conversion not completed yet!");
                memset(response, 0xFF, length);
                break;
            }
            // Format cell voltage response for 16 cells
            for (int i = 0; i < TOTAL_MODULES; i++) {
                for (int j = 0; j < CELLS_PER_DEVICE; j++) {
                    response[i * CELLS_PER_DEVICE * 2 + j * 2] = 
                        (device_registers[i].cell_voltages[j] >> 8) & 0xFF;
                    response[i * CELLS_PER_DEVICE * 2 + j * 2 + 1] = 
                        device_registers[i].cell_voltages[j] & 0xFF;
                }
            }
            break;
            
        case RDAUXA:
        case RDAUXB:
            ESP_LOGI(TAG, "Read Auxiliary Register Command: 0x%02X", command);
            for (int i = 0; i < TOTAL_MODULES; i++) {
                memcpy(&response[i * AUX_REG_SIZE], device_registers[i].aux_reg, AUX_REG_SIZE);
            }
            break;
            
        case RDSTATA:
        case RDSTATB:
            ESP_LOGI(TAG, "Read Status Register Command: 0x%02X", command);
            for (int i = 0; i < TOTAL_MODULES; i++) {
                memcpy(&response[i * STAT_REG_SIZE], device_registers[i].status_reg, STAT_REG_SIZE);
            }
            break;
            
        case WRCFGA:
        case WRCFGB:
            ESP_LOGI(TAG, "Write Configuration Register Command: 0x%02X", command);
            for (int i = 0; i < TOTAL_MODULES; i++) {
                memcpy(device_registers[i].config_reg, &cmd[2], CFG_REG_SIZE);
            }
            memset(response, 0, length);
            break;
            
        case RDCFGA:
        case RDCFGB:
            ESP_LOGI(TAG, "Read Configuration Register Command: 0x%02X", command);
            for (int i = 0; i < TOTAL_MODULES; i++) {
                memcpy(&response[i * CFG_REG_SIZE], device_registers[i].config_reg, CFG_REG_SIZE);
            }
            break;
            
        case PLADC:
            ESP_LOGI(TAG, "Poll ADC Status Command");
            response[0] = adc_busy ? 0xFF : 0x00;
            break;
            
        case DIAGN:
            ESP_LOGI(TAG, "Diagnostic Command");
            // Set status register bits based on diagnostic results
            for (int i = 0; i < TOTAL_MODULES; i++) {
                device_registers[i].status_reg[0] = 0x00; // All OK
            }
            memset(response, 0, length);
            break;
            
        default:
            ESP_LOGW(TAG, "Unknown Command: 0x%02X", command);
            memset(response, 0xFF, length);
            break;
    }
    
    // Add PEC to response
    response[length - 1] = calculate_pec(response, length - 1);
}

// SPI slave task
void spi_slave_task(void *arg) {
    uint8_t recv_buffer[4] = {0};
    uint8_t send_buffer[300] = {0};

    spi_slave_transaction_t trans = {
        .length = sizeof(recv_buffer) * 8,
        .tx_buffer = send_buffer,
        .rx_buffer = recv_buffer
    };

    while (1) {
        if (spi_slave_transmit(SPI_HOST, &trans, portMAX_DELAY) == ESP_OK) {
            handle_spi_command(recv_buffer, send_buffer, sizeof(send_buffer));
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

// Initialize SPI slave
void init_spi_slave() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SPI_MOSI) | (1ULL << SPI_MISO) | 
                        (1ULL << SPI_CLK) | (1ULL << SPI_CS),
        .mode = GPIO_MODE_INPUT_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
        .flags = 0,
        .intr_flags = 0
    };

    spi_slave_interface_config_t slvcfg = {
        .mode = 0,
        .spics_io_num = SPI_CS,
        .queue_size = 1,
        .flags = 0,
        .post_setup_cb = NULL,
        .post_trans_cb = NULL
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_slave_initialize(SPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO));
}

void app_main() {
    ESP_LOGI(TAG, "Starting ADBMS6830 Simulator...");
    init_device_registers();
    init_spi_slave();
    
    xTaskCreate(spi_slave_task, "spi_slave_task", 4096, NULL, 10, NULL);
    
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
