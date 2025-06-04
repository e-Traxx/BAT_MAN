/*
 * flash_storage.c
 * Non-volatile storage via I²C FRAM (FM24V05-G)
 */

#include "flash_storage.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

static const char *TAG = "flash_storage";

/* I²C master configuration */
#define I2C_MASTER_NUM            I2C_NUM_0
#define I2C_MASTER_SDA_IO         GPIO_NUM_10
#define I2C_MASTER_SCL_IO         GPIO_NUM_9
#define I2C_MASTER_FREQ_HZ        400000    /*!< 400 kHz */
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

/* FM24V05-G I²C address (A2,A1,A0 = VCC) */
#define FRAM_I2C_ADDR             0x57

static bool is_initialized = false;

/* Initialize I²C peripheral */
static esp_err_t i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = I2C_MASTER_SDA_IO,
        .scl_io_num       = I2C_MASTER_SCL_IO,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(
        I2C_MASTER_NUM,
        I2C_MODE_MASTER,
        I2C_MASTER_RX_BUF_DISABLE,
        I2C_MASTER_TX_BUF_DISABLE,
        0
    ));
    ESP_LOGI(TAG, "I²C master initialized (SDA=%d, SCL=%d)", I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO);
    return ESP_OK;
}

/* Low-level FRAM write at 16-bit address */
static esp_err_t fram_write(uint16_t mem_addr, const uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Device address + write bit
    i2c_master_write_byte(cmd, (FRAM_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    // 16-bit memory address (MSB first)
    i2c_master_write_byte(cmd, (uint8_t)(mem_addr >> 8),    true);
    i2c_master_write_byte(cmd, (uint8_t)(mem_addr & 0xFF),  true);
    // Payload
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

/* Low-level FRAM read from 16-bit address */
static esp_err_t fram_read(uint16_t mem_addr, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    // Set address pointer
    i2c_master_write_byte(cmd, (FRAM_I2C_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, (uint8_t)(mem_addr >> 8),    true);
    i2c_master_write_byte(cmd, (uint8_t)(mem_addr & 0xFF),  true);
    // Restart for read
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (FRAM_I2C_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, data, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);
    return ret;
}

/* Public API */

esp_err_t flash_storage_init(void)
{
    if (is_initialized) {
        return ESP_OK;
    }
    esp_err_t ret = i2c_master_init();
    if (ret == ESP_OK) {
        is_initialized = true;
        ESP_LOGI(TAG, "FRAM storage initialized");
    }
    return ret;
}

bool flash_storage_is_initialized(void)
{
    return is_initialized;
}

esp_err_t flash_storage_erase_config(void)
{
    // FRAM does not require erase cycles.
    // To “clear” a config region, manually write 0x00 over it if needed.
    return ESP_OK;
}

esp_err_t flash_read_data(uint32_t addr, uint8_t *buf, size_t len)
{
    if (!is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    return fram_read((uint16_t)addr, buf, len);
}

esp_err_t flash_write_page(uint32_t addr, const uint8_t *data, size_t len)
{
    if (!is_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    return fram_write((uint16_t)addr, data, len);
}