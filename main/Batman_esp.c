#include "Batman_esp.h"
#include "Albert_handler.h"
#include "OTA.h"
#include "Robin_handler.h"
#include "Wifi.h"
#include "can_handler.h"
#include "diagnostic_handler.h"
#include "driver/twai.h" // TWAI is Espressif's CAN driver in ESP-IDF
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/idf_additions.h"
#include "freertos/task.h"
#include "spi_handler.h"
#include <stdio.h>

static const char *TAG = "Setup";

// Overall System Start Status
bool SystemInitialised;

// Flags to check functionality of System and avoid Stackoverflow due to too
// many errors
System_health_Flags flags;

/*
 * Startup Procedure
 *   CAN Setup()
 * - Configure Network
 * - Configure Semaphore for CAN Transmission
 *
 *   SPI Setup()
 * - Detect ADBMS6822 module
 * - Setup connection to module and configure network.
 *
 *
 *   ADBMS Setup()
 * 1. Poll all cell modules for Serial IDs
 * 2. Store all Serial IDs in a 14 index array.
 * 3. Send Command to start ADC Measurements on the Modules.
 *
 *
 *   DIAG Setup()
 * 1. Create Semaphore and Timer.
 * 2. Gives semaphore every 200ms
 *
 *
 *   WIFI Setup()
 * 1. Setup Wifi Connection
 * 2. Setup DHCP
 * 3. Setup Static Connection
 * 4. Start Wifi AP
 *
 *
 *  Albert_Setup()
 * 1. Setup ADC for Current Measurement
 * 2. Check if Sensor is connected
 * 3. Create Semaphore and Timer
 *
 *
 */

void
initialise_setups (void)
{
    // Setup

    // CAN_Setup();
    ISOSPI_Setup ();
    // WIFI_Setup();

    // Setup Diagnostic system
    // Diag_Setup();
    ADBMS_Setup ();
    // Albert_Setup();

    unsigned int system_health = *(unsigned int *)&flags;
    if (system_health)
	{
	    // If at least one flag is set, then
	    SystemInitialised = false;
	}
}

void
Start_schedule (void)
{

    // // Start Timer scheduling
    // if (xTimerStart(diagnostic_timer, 0) != pdPASS) {
    //   ESP_LOGE(TAG, "DIAG Timer create failed – heap %u",
    //            esp_get_free_heap_size());
    //   ESP_LOGE(TAG, "[-] Failed to start diagnostic_timer\n");
    // }

    if (xTimerStart (adbms_timer, 0) != pdPASS)
	{
	    ESP_LOGE (TAG, "ROBIN Timer create failed – heap %" PRIu32, esp_get_free_heap_size ());
	    ESP_LOGE (TAG, "[-] Failed to start adbms_timer");
	}
    //
    // if (xTimerStart(albert_timer, 0) != pdPASS) {
    //   ESP_LOGE(TAG, "ALBERT Timer create failed – heap %u",
    //            esp_get_free_heap_size());
    //   ESP_LOGE(TAG, "[-] Failed to start albert_timer");
    // }

    // Schedule a Robin query every 200ms (test case) // can be higher frequency
    //
    // Upon created, tasks are placed into ready state
    // xTaskCreate(Diagnostic_check, "System_diag", 4096, NULL, 2, NULL);
    // xTaskCreate(CAN_sendMessage, "Can_tx", 2048, NULL, 3, NULL);
    xTaskCreate (Robin_query, "Robin_query", 8192, NULL, 2, NULL);
    // xTaskCreate(Albert_Query, "Albert_query", 2048, NULL, 1, NULL);
}

void
app_main (void)
{

    ESP_LOGI (TAG, "heap before timers: %" PRIu32, esp_get_free_heap_size ());

    initialise_setups ();
    if (SystemInitialised)
	{
	    ESP_LOGI (TAG, "[+] Startup Process Completed\n");
	}
    else
	{
	    ESP_LOGW (TAG, "[@] Startup Process Completed with Errors\n");
	}
    Start_schedule ();

    uxTaskGetStackHighWaterMark (NULL);
}
