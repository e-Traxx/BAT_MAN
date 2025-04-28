#include "flash_storage.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>

static const char *TAG = "Flash Storage";

// Flash memory commands
#define FLASH_CMD_WRITE_ENABLE    0x06
#define FLASH_CMD_WRITE_DISABLE   0x04
#define FLASH_CMD_READ_STATUS     0x05
#define FLASH_CMD_WRITE_STATUS    0x01
#define FLASH_CMD_READ_DATA       0x03
#define FLASH_CMD_PAGE_PROGRAM    0x02
#define FLASH_CMD_SECTOR_ERASE    0x20
#define FLASH_CMD_CHIP_ERASE      0xC7

// Flash memory status register bits
#define FLASH_STATUS_BUSY         0x01
#define FLASH_STATUS_WRITE_EN     0x02

// Configuration storage address
#define CONFIG_STORAGE_ADDRESS    0x000000

static spi_device_handle_t flash_handle;
static bool is_initialized = false;

// Helper functions
static esp_err_t flash_wait_for_ready(void) {
    uint8_t status;
    spi_transaction_t t = {
        .cmd = FLASH_CMD_READ_STATUS,
        .rxlength = 8,
        .rx_buffer = &status
    };

    do {
        ESP_ERROR_CHECK(spi_device_transmit(flash_handle, &t));
    } while (status & FLASH_STATUS_BUSY);

    return ESP_OK;
}

static esp_err_t flash_write_enable(void) {
    spi_transaction_t t = {
        .cmd = FLASH_CMD_WRITE_ENABLE
    };
    return spi_device_transmit(flash_handle, &t);
}

static esp_err_t flash_write_disable(void) {
    spi_transaction_t t = {
        .cmd = FLASH_CMD_WRITE_DISABLE
    };
    return spi_device_transmit(flash_handle, &t);
}

// Public functions
esp_err_t flash_storage_init(void) {
    if (is_initialized) {
        return ESP_OK;
    }

    // Configure SPI bus
    spi_bus_config_t bus_config = {
        .miso_io_num = 12,  // Customize based on your hardware
        .mosi_io_num = 13,  // Customize based on your hardware
        .sclk_io_num = 14,  // Customize based on your hardware
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096
    };

    ESP_ERROR_CHECK(spi_bus_initialize(FLASH_SPI_HOST, &bus_config, SPI_DMA_CH_AUTO));

    // Configure SPI device
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = FLASH_CLOCK_SPEED,
        .mode = 0,
        .spics_io_num = FLASH_CS_PIN,
        .queue_size = 7,
        .command_bits = 8,
        .address_bits = 24,
        .dummy_bits = 0
    };

    ESP_ERROR_CHECK(spi_bus_add_device(FLASH_SPI_HOST, &dev_config, &flash_handle));

    is_initialized = true;
    ESP_LOGI(TAG, "Flash storage initialized");
    return ESP_OK;
}

esp_err_t flash_storage_read_config(system_config_t *config) {
    if (!is_initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }

    spi_transaction_t t = {
        .cmd = FLASH_CMD_READ_DATA,
        .addr = CONFIG_STORAGE_ADDRESS,
        .length = sizeof(system_config_t) * 8,
        .rx_buffer = config
    };

    return spi_device_transmit(flash_handle, &t);
}

esp_err_t flash_storage_write_config(const system_config_t *config) {
    if (!is_initialized || !config) {
        return ESP_ERR_INVALID_ARG;
    }

    // Enable write
    ESP_ERROR_CHECK(flash_write_enable());

    // Erase the sector first
    spi_transaction_t erase_t = {
        .cmd = FLASH_CMD_SECTOR_ERASE,
        .addr = CONFIG_STORAGE_ADDRESS
    };
    ESP_ERROR_CHECK(spi_device_transmit(flash_handle, &erase_t));
    ESP_ERROR_CHECK(flash_wait_for_ready());

    // Enable write again after erase
    ESP_ERROR_CHECK(flash_write_enable());

    // Write the configuration
    spi_transaction_t write_t = {
        .cmd = FLASH_CMD_PAGE_PROGRAM,
        .addr = CONFIG_STORAGE_ADDRESS,
        .length = sizeof(system_config_t) * 8,
        .tx_buffer = config
    };
    ESP_ERROR_CHECK(spi_device_transmit(flash_handle, &write_t));
    ESP_ERROR_CHECK(flash_wait_for_ready());

    // Disable write
    ESP_ERROR_CHECK(flash_write_disable());

    return ESP_OK;
}

esp_err_t flash_storage_erase_config(void) {
    if (!is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_ERROR_CHECK(flash_write_enable());

    spi_transaction_t t = {
        .cmd = FLASH_CMD_SECTOR_ERASE,
        .addr = CONFIG_STORAGE_ADDRESS
    };

    ESP_ERROR_CHECK(spi_device_transmit(flash_handle, &t));
    ESP_ERROR_CHECK(flash_wait_for_ready());
    ESP_ERROR_CHECK(flash_write_disable());

    return ESP_OK;
}

bool flash_storage_is_initialized(void) {
    return is_initialized;
} 