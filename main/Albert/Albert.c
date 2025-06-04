#include "Batman_esp.h"
#include "Robin_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG = "Albert";

// ADC Configuration
#define ADC_UNIT ADC_UNIT_1
#define ADC_CHAN ADC_CHANNEL_1
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BIT_WIDTH ADC_BITWIDTH_12

SemaphoreHandle_t albert_semaphore;
TimerHandle_t albert_timer;

// ADC Driver Handles
static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t cali_handle;

/*
 * Current Measurement Process:
 * - The HTFS-200p sensor is used to measure the current flowing through the
 *   TSAC main HV wires.
 * - A user-defined table maps sensor voltage values to corresponding current
 *   values.
 * - Linear interpolation is applied to calculate the current based on the
 *   sensor's output.
 * - The measured current is logged and stored for further processing.
 */

void Albert_timer_callback(TimerHandle_t timer) {
  BaseType_t HighPriorityTaskWoken = pdFALSE;

  // Give Semaphore to Albert_task
  if (xSemaphoreGiveFromISR(albert_semaphore, &HighPriorityTaskWoken) !=
      pdPASS) {
    ESP_LOGE(TAG, "Failed to give Semaphore to Albert_task");
  }
}

void Albert_Setup() {
  /* Check if the Sensor is Connected / Check for Open Wire
   * The sensor always lets about 1.25V through, so if the sensor is
   * disconnected, the voltage will be 0V or lower than 2.5V.
   */

  // Initialize ADC in Oneshot Mode
  adc_oneshot_unit_init_cfg_t unit_cfg = {
      .unit_id = ADC_UNIT,
  };
  if (adc_oneshot_new_unit(&unit_cfg, &adc_handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize ADC unit.");
    return;
  }

  // Configure ADC Channel
  adc_oneshot_chan_cfg_t chan_cfg = {
      .bitwidth = ADC_BIT_WIDTH,
      .atten = ADC_ATTEN,
  };
  if (adc_oneshot_config_channel(adc_handle, ADC_CHAN, &chan_cfg) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure ADC channel.");
    return;
  }

  // Initialize Calibration (if supported)
  adc_cali_curve_fitting_config_t cali_cfg = {
      .unit_id = ADC_UNIT,
      .atten = ADC_ATTEN,
      .bitwidth = ADC_BIT_WIDTH,
  };
  if (adc_cali_create_scheme_curve_fitting(&cali_cfg, &cali_handle) != ESP_OK) {
    ESP_LOGW(TAG,
             "ADC calibration not available. Falling back to raw estimation.");
    cali_handle = NULL;
  }

  // Check if the Sensor is Connected / Check for Open Wire
  int raw = 0, voltage = 0;
  adc_oneshot_read(adc_handle, ADC_CHAN, &raw);
  if (cali_handle) {
    adc_cali_raw_to_voltage(cali_handle, raw, &voltage);
  } else {
    voltage = raw * 3300 / 4095; // Fallback conversion for 3.3V system
  }

  if (voltage < 1100) {
    ESP_LOGE(TAG, "Sensor is not connected or there is an open wire.");
  }

  // Initialise the Semaphore
  albert_semaphore = xSemaphoreCreateBinary();
  if (albert_semaphore == NULL) {
    ESP_LOGE(TAG, "Failed to create semaphore.");
    return;
  }

  // Create Timer
  albert_timer = xTimerCreate("Albert_timer", pdMS_TO_TICKS(200), pdTRUE,
                              (void *)0, Albert_timer_callback);
  if (albert_timer == NULL) {
    ESP_LOGE(TAG, "Failed to create timer.");
    return;
  }

  xTimerStart(albert_timer, 0);
}

void Albert_Query(void *args) {
  while (1) {
    if (xSemaphoreTake(albert_semaphore, portMAX_DELAY) == pdTRUE) {
      // Perform the diagnostic test
      ESP_LOGV(TAG, "[*] Measuring current...");

      // Read the Current
      int raw = 0, voltage = 0;
      if (adc_oneshot_read(adc_handle, ADC_CHAN, &raw) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read from ADC.");
        continue;
      }

      if (cali_handle) {
        adc_cali_raw_to_voltage(cali_handle, raw, &voltage);
      } else {
        voltage = raw * 3300 / 4095;
      }

      // Interpolation Equation
      // At 0A, the sensor outputs 1.25V
      // At 10A, the sensor outputs 1.3125V
      // At 25A, the sensor outputs 1.46875V
      // At 50A, the sensor outputs 1.71875V
      // At 100A, the sensor outputs 2.1875V
      // At 200A, the sensor outputs 3.125V

      // Linear Interpolation
      // y = mx + c
      // V = 0.0094I + 1.25
      // I = (V - 1.25) / 0.0094
      if (voltage < 1100) {
        ESP_LOGE(TAG, "Sensor is not connected or there is an open wire.");
        flags.ALBERT_Error = 1; // Set the error flag
        continue;
      }
      float current = (voltage - 1250.0f) / 9.4f;
      if (current < 0 && flags.Charging) {
        current = current * -1;
      }
      // Store the current in the shared struct
      robin->overall_current = (uint16_t)current;

      // ESP_LOGI(TAG, "Current = %.2f A | Voltage = %d mV | Raw = %d", current,
      // voltage, raw);
    }
  }
}
