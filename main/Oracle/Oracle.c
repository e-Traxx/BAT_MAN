#include "OTA.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "diagnostic_handler.h"
#include "Robin_handler.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "ORACLE";

// Global pointers to data containers
extern Diagnostic_Container_t *g_diagnostic_data;
extern Robin_container_t *g_robin_data;

// Handler for diagnostic data endpoint
// Returns JSON containing overall battery status and error flags
static esp_err_t get_diagnostic_data(httpd_req_t *req) {
    // Create JSON response
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON");
        return ESP_FAIL;
    }

    // Get diagnostic data from global pointer
    if (g_diagnostic_data) {
        // Add overall measurements
        cJSON_AddNumberToObject(root, "overall_voltage", g_diagnostic_data->overall_voltage);
        cJSON_AddNumberToObject(root, "current", g_diagnostic_data->current);
        cJSON_AddNumberToObject(root, "temperature", g_diagnostic_data->temp);
        cJSON_AddNumberToObject(root, "soc", g_diagnostic_data->soc);
        cJSON_AddNumberToObject(root, "soh", g_diagnostic_data->soh);
        
        // Add error flags as individual boolean values
        cJSON *flags = cJSON_CreateObject();
        cJSON_AddBoolToObject(flags, "lost_comm", g_diagnostic_data->flags[0] & 0x01);
        cJSON_AddBoolToObject(flags, "voltage_sense_error", g_diagnostic_data->flags[0] & 0x02);
        cJSON_AddBoolToObject(flags, "overall_overvoltage", g_diagnostic_data->flags[0] & 0x04);
        cJSON_AddBoolToObject(flags, "balancing_on", g_diagnostic_data->flags[0] & 0x08);
        cJSON_AddBoolToObject(flags, "temp_sensor_loss", g_diagnostic_data->flags[0] & 0x10);
        cJSON_AddBoolToObject(flags, "pack_undervoltage", g_diagnostic_data->flags[0] & 0x20);
        cJSON_AddBoolToObject(flags, "current_sensor_loss", g_diagnostic_data->flags[0] & 0x40);
        cJSON_AddBoolToObject(flags, "high_temp", g_diagnostic_data->flags[0] & 0x80);
        cJSON_AddItemToObject(root, "flags", flags);
    } else {
        // If no data available, return empty object
        ESP_LOGE(TAG, "No diagnostic data available");
    }

    // Convert JSON object to string for transmission
    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON string");
        return ESP_FAIL;
    }

    // Set content type and send response
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);

    // Cleanup allocated memory
    free(json_str);
    cJSON_Delete(root);
    return ESP_OK;
}

// Handler for cell data endpoint
// Returns detailed voltage and temperature measurements for each cell
static esp_err_t get_cell_data(httpd_req_t *req) {
    cJSON *root = cJSON_CreateObject();
    if (!root) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON");
        return ESP_FAIL;
    }

    // Get Robin data from global pointer
    if (g_robin_data) {
        // Add overall measurements from Robin container
        cJSON_AddNumberToObject(root, "overall_voltage", g_robin_data->overall_voltage);
        cJSON_AddNumberToObject(root, "overall_temperature", g_robin_data->overall_temperature);
        cJSON_AddNumberToObject(root, "overall_current", g_robin_data->overall_current);
        cJSON_AddNumberToObject(root, "overall_SOC", g_robin_data->overall_SOC);

        // Create nested arrays for cell voltages
        // Structure: [stack1[cell1, cell2, ...], stack2[cell1, cell2, ...], ...]
        cJSON *cell_voltages = cJSON_CreateArray();
        for (int stack = 0; stack < NUM_STACKS; stack++) {
            cJSON *stack_voltages = cJSON_CreateArray();
            for (int cell = 0; cell < CELLS_PER_STACK_ACTIVE; cell++) {
                cJSON_AddItemToArray(stack_voltages, cJSON_CreateNumber(g_robin_data->individual_voltages[stack][cell]));
            }
            cJSON_AddItemToArray(cell_voltages, stack_voltages);
        }
        cJSON_AddItemToObject(root, "cell_voltages", cell_voltages);

        // Create nested arrays for cell temperatures
        // Structure: [stack1[cell1, cell2, ...], stack2[cell1, cell2, ...], ...]
        cJSON *cell_temperatures = cJSON_CreateArray();
        for (int stack = 0; stack < NUM_STACKS; stack++) {
            cJSON *stack_temps = cJSON_CreateArray();
            for (int cell = 0; cell < CELLS_PER_STACK_ACTIVE; cell++) {
                cJSON_AddItemToArray(stack_temps, cJSON_CreateNumber(g_robin_data->individual_temperatures[stack][cell]));
            }
            cJSON_AddItemToArray(cell_temperatures, stack_temps);
        }
        cJSON_AddItemToObject(root, "cell_temperatures", cell_temperatures);
    } else {
        // If no data available, return empty object
        ESP_LOGE(TAG, "No Robin data available");
    }

    // Convert JSON object to string for transmission
    char *json_str = cJSON_PrintUnformatted(root);
    if (!json_str) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create JSON string");
        return ESP_FAIL;
    }

    // Set content type and send response
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, json_str);

    // Cleanup allocated memory
    free(json_str);
    cJSON_Delete(root);
    return ESP_OK;
}

// Reset endpoint handler
static esp_err_t reset_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "Received system reset request");
    
    // Send response before resetting
    const char *response = "{\"status\":\"restarting\"}";
    httpd_resp_send(req, response, strlen(response));
    
    // Give some time for the response to be sent
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Perform full system restart
    esp_restart();
    
    return ESP_OK;
}

// Register URI handlers for the HTTP server
// Sets up endpoints for diagnostic and cell data retrieval
void Oracle_Setup(httpd_handle_t server) {
    // Diagnostic data endpoint configuration
    httpd_uri_t diagnostic_uri = {
        .uri = "/api/diagnostic",    // Endpoint path
        .method = HTTP_GET,          // HTTP method
        .handler = get_diagnostic_data, // Handler function
        .user_ctx = NULL            // No user context needed
    };

    // Cell data endpoint configuration
    httpd_uri_t cell_data_uri = {
        .uri = "/api/cell_data",     // Endpoint path
        .method = HTTP_GET,          // HTTP method
        .handler = get_cell_data,    // Handler function
        .user_ctx = NULL            // No user context needed
    };

    // Register both endpoints with the HTTP server
    httpd_register_uri_handler(server, &diagnostic_uri);
    httpd_register_uri_handler(server, &cell_data_uri);

    // Register reset endpoint
    httpd_uri_t reset_uri = {
        .uri = "/api/reset",     // Endpoint path
        .method = HTTP_POST,     // HTTP method
        .handler = reset_handler,// Handler function
        .user_ctx = NULL        // No user context needed
    };
    
    httpd_register_uri_handler(server, &reset_uri);
}
