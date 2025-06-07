#include "diagnostic_handler.h"
#include "Batman_esp.h"
#include "Robin_handler.h"
#include "can_handler.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/projdefs.h"
#include "spi_handler.h"
#include <stdint.h>
#include <string.h>

SemaphoreHandle_t diagnostic_semaphore;
TimerHandle_t diagnostic_timer;

// --- Diagnostic Values ---
int overvoltage_limit = 4150;
int undervoltage_limit = 3000;

int hightemp_limit = 550;
int lowtemp_limit = 100;

static const char *TAG = "Diag";

// Commands
#define STATC 0x790

Diagnostic_Container_t diag;
static diag_frame_t frame;

// Function Definitions
void diagnostic_timer_callback(TimerHandle_t DiagTimer);
void Diagnostic_check(void *arguments);
void Diagnostic_packet_formatter(diag_frame_t *frame, uint16_t voltage,
                                 uint16_t current, uint16_t temp, uint8_t soc,
                                 uint8_t soh, uint8_t flags[8]);
void Check_connections_and_limits(uint8_t *flags);
void Check_limits(uint8_t *flags);
void Fault_management(uint8_t *Flags);

void Diag_Setup(void) {
  // Semaphore Setup
  diagnostic_semaphore = xSemaphoreCreateBinary();
  if (diagnostic_semaphore == NULL) {
    printf("[-] Failed to create semaphore.\n");
    return;
  }

  // Create Timer
  diagnostic_timer = xTimerCreate("Diagnostic_timer", pdMS_TO_TICKS(1000),
                                  pdTRUE, (void *)0, diagnostic_timer_callback);
  if (diagnostic_timer == NULL) {
    printf("[-] Failed to create timer.\n");
    return;
  }

  // System Initialised
  diag.overall_voltage = 0;
  diag.temp = 0;
  diag.current = 0;
  diag.soc = 0;
  diag.soh = 0;

  // Error Flags --- See Header File for Format
  memset(diag.flags, 0, sizeof(diag.flags));
}

// Gives the Semaphore every 200ms
void diagnostic_timer_callback(TimerHandle_t DiagTimer) {
  BaseType_t HighPriorityTaskWoken = pdFALSE;

  // Give Semaphore to Diagnostic_task
  if (xSemaphoreGiveFromISR(diagnostic_semaphore, &HighPriorityTaskWoken) !=
      pdPASS) {
    printf("[-] Failed to give Semaphore to Diagnostic_check\n");
  }
}

// Procedure:
//  1. Poll the Status Register and check for Open Wire
//  2. Check the Current Sensor for Open Wire
//  3. Check the Voltage Sense and Temperature Sensing for Open Wire
//  4. Check that the Voltage and Temperature Values are within limits
//  5. Determine SOC of Battery
//  6. Check Power delivery of Battery
void Diagnostic_check(void *arguments) {
  UBaseType_t uxHighWaterMark;
  
  while (1) {
    if (xSemaphoreTake(diagnostic_semaphore, portMAX_DELAY) == pdTRUE) {
      // Get stack high water mark - shows minimum free stack space during task execution
      uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
      // ESP_LOGI(TAG, "Stack high water mark: %u", uxHighWaterMark);
      
      // Perform the diagnostic test
      ESP_LOGV(TAG, "[*] Running diagnostic test...");

      // Check Sensors Loss and limits
      ESP_LOGV(TAG, "[*]  Checking Limits");
      Check_connections_and_limits(diag.flags);
      ESP_LOGV(TAG, "[*] Fault_management");
      Fault_management(diag.flags);
      ESP_LOGV(TAG, "[*] Packet Formatter");
      Diagnostic_packet_formatter(&frame, diag.overall_voltage, diag.current,
                                diag.temp, diag.soc, diag.soh, diag.flags);

      // enqueue data to CAN_TX send function
      CAN_TX_enqueue(0x199, 8, frame.bytes);

      // for now
      ESP_LOGV(TAG, "[+] Diagnostic test completed.\n");
    }
  }
}

/*   uint32_t lost_comm : 1;           // Bit 55 */
/*   uint32_t Voltage_sense_error : 1; // Bit 56 */
/*   uint32_t Overall_overvoltage : 1; // Bit 57 */
/*   uint32_t Balancing_on : 1;        // Bit 58 */
/*   uint32_t temp_sensor_loss : 1;    // Bit 59 */
/*   uint32_t pack_undervoltage : 1;   // Bit 60 */
/*   uint32_t Curr_sensor_loss : 1;    // Bit 61 */
/*   uint32_t high_temp : 1;    */

// Diagnostic Checks
void Check_connections_and_limits(uint8_t *flags) {
  if (!SystemInitialised) {
    ESP_LOGW(TAG, "Skipping Diagnostic Checks........");
    return;
  }
  // Current Sensor = Voltage Measured = 0V

  // Using Open Wire Sensing (Which is set up in the Start Measurement Command
  // in spi_handler.c )
  // if the Voltage Measured is 0V then we have an OPEN WIRE.
  /* uint8_t cell_index = 0; */
  for (int Stack = 0; Stack < NUM_STACKS; Stack++) {
    for (int Cell = 0; Cell < CELLS_PER_STACK_ACTIVE; Cell++) {
      // ---- LIMITS CHECK ----
      // Voltage
      // Undervoltage: as from 3.0V
      // Overvoltage: as from 4.15V
      //
      // To avoid rapid on and off, we check if normal Status is restored before
      // resuming.
      if (robin->individual_voltages[Stack][Cell] > overvoltage_limit) {
        // Add Flag
        ESP_LOGE(TAG, "Overvoltage Detected Stack: %d, Cell: %d", Stack, Cell);
      }

      if (robin->individual_voltages[Stack][Cell] < undervoltage_limit) {
        // Add Flag
        ESP_LOGE(TAG, "Undervoltage Detected Stack: %d, Cell: %d", Stack, Cell);
      }

      // ---- LIMITS CHECK ----
      // Temperature
      // High Temperature Limit: as from 55°C
      // Low Temperature Limit: as from 10°C
      if (robin->individual_temperatures[Stack][Cell] > hightemp_limit) {
        // Add Flag
        ESP_LOGE(TAG, "High Temperature Detected Stack: %d, Cell: %d", Stack,
                 Cell);
      }

      if (robin->individual_voltages[Stack][Cell] < lowtemp_limit) {
        // Add Flag
        ESP_LOGE(TAG, "Low Temperature Detected Stack: %d, Cell: %d", Stack,
                 Cell);
      }
    }
  }
}

// Manages Fault based on the Flags set
// Using a Hysteresis System, the System is allowed to restore normal
// Functioning Conditions before resuming functionalities
void Fault_management(uint8_t *flags) {}

// Coulomb Counting and OCV (for reset)
void SOC() {}

void Diagnostic_packet_formatter(diag_frame_t *frame, uint16_t voltage,
                                 uint16_t current, uint16_t temp, uint8_t soc,
                                 uint8_t soh, uint8_t flags[8]) {

  // Direct bit assignments matching DBC file exactly
  frame->fields.Overall_voltage = voltage & 0x1FFF;        // 0|13@1+ (0.1,0) [0|819.1] "V"
  frame->fields.Highest_temp_recorded = temp & 0x3FFF;     // 13|14@1+ (0.01,0) [0|163.83] "∞C"
  frame->fields.Curr_value = current & 0xFFF;              // 27|12@1+ (0.1,0) [0|409.5] "A"
  frame->fields.SOC = soc;                                 // 39|8@1- (1,0) [0|0]
  
  // Error Flags (exactly matching DBC bit positions)
  frame->fields.lost_comm = (flags[0] & 0x1);              // 47|1@1- (1,0) [0|0]
  frame->fields.Voltage_sensor_loss = (flags[1] & 0x1);    // 48|1@1- (1,0) [0|0]
  frame->fields.Battery_overvoltage = (flags[2] & 0x1);    // 49|1@1- (1,0) [0|0]
  frame->fields.Balancing_on = (flags[3] & 0x1);           // 50|1@1- (1,0) [0|0]
  frame->fields.temp_sensor_loss = (flags[4] & 0x1);       // 51|1@1- (1,0) [0|0]
  frame->fields.Battery_undervoltage = (flags[5] & 0x1);   // 52|1@1- (1,0) [0|0]
  frame->fields.curr_sensor_loss = (flags[6] & 0x1);       // 53|1@1- (1,0) [0|0]
  frame->fields.Over_templimit = (flags[7] & 0x1);         // 54|1@1- (1,0) [0|0]
  frame->fields.System_health = 0;                         // 55|1@1+ (1,0) [0|1]
  
  frame->fields.Reserved = 0;                              // Remaining bits
}
