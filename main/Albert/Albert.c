#include "Albert_handler.h"
#include "Batman_esp.h"
#include "Robin_handler.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"

static const char *TAG = "Albert";

#define ADC_CHAN ADC1_CHANNEL_1

SemaphoreHandle_t albert_semaphore;
TimerHandle_t albert_timer;
// Characterize ADC
esp_adc_cal_characteristics_t adc_chars;
void Albert_timer_callback(TimerHandle_t timer);
// Measures Current flowing through the TSAC main HV Wires

/*
 * Current Measurement Process:
 * - The HTFS-200p sensor is used to measure the current flowing through the
 * TSAC main HV wires.
 * - A user-defined table maps sensor voltage values to corresponding current
 * values.
 * - Linear interpolation is applied to calculate the current based on the
 * sensor's output.
 * - The measured current is logged and stored for further processing.
 */

void Albert_Setup() {
  /* Check if the Sensor is Connected / Check for Open Wire
   * The sensor always lets about 1.25V through, so if the sensor is
   * disconnected, the voltage will be 0V or lower than 2.5V.
   */

  // 12-Bit value is used (0 - 4096)
  adc1_config_width(ADC_WIDTH_BIT_12);
  // Set the attenuation to 11dB to allow for a full-scale voltage of 3.9V
  adc1_config_channel_atten(ADC_CHAN, ADC_ATTEN_DB_12);
  // Default Vref is 1100mV
  esp_adc_cal_value_t V_conf = esp_adc_cal_characterize(
      ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);

  // Check if the Sensor is Connected / Check for Open Wire
  int adc_reading = adc1_get_raw(ADC_CHAN);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);

  // If the voltage is lower than 1.25V, the sensor is not connected or there is
  // an open wire
  if (voltage < 1000) {
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
}

void Albert_timer_callback(TimerHandle_t AlbertTimer) {
  BaseType_t HighPriorityTaskWoken = pdFALSE;

  // Give Semaphore to Albert_task
  if (xSemaphoreGiveFromISR(albert_semaphore, &HighPriorityTaskWoken) !=
      pdPASS) {
    ESP_LOGE(TAG, "Failed to give Semaphore to Albert_task");
  }
}

void Albert_Query(void *args) {
  while (1) {
    if (xSemaphoreTake(albert_semaphore, portMAX_DELAY) == pdTRUE) {
      // Perform the diagnostic test
      ESP_LOGV(TAG, "[*] Measuring current...");

      // Read the Current
      int adc_reading = adc1_get_raw(ADC_CHAN);
      uint16_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, &adc_chars);
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
      uint16_t current = (voltage - 1250) / 9.4;
      robin->overall_current = current;

      ESP_LOGV(TAG, "[+] Current Measurement: %d.\n", current);
    }
  }
}
