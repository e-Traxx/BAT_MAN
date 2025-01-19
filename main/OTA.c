#include "OTA.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_https_server.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "OTA";

#define MIN(x, y) ((x) < (y) ? (x) : (y))

esp_err_t Ota_updater(httpd_req_t *req) {
  // Suspend normal funtion

  // Perform OTA Update
  esp_ota_handle_t ota_handle;
  const esp_partition_t *ota_partition =
      esp_ota_get_next_update_partition(NULL);
  esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);

  char buf[1024];
  int remaining = req->content_len;
  while (remaining > 0) {
    int received = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));
    if (received <= 0)
      break;

    esp_ota_write(ota_handle, buf, received);
    remaining -= received;
  }

  esp_ota_end(ota_handle);
  esp_ota_set_boot_partition(ota_partition);
  esp_restart();

  // Finalize the OTA update
  if (esp_ota_end(ota_handle) != ESP_OK) {
    ESP_LOGE(TAG, "OTA End Failed");
    return ESP_FAIL;
  }

  // Set the updated partition as boot partition
  if (esp_ota_set_boot_partition(ota_partition) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set boot partition");
    return ESP_FAIL;
  }

  // Restart the device
  ESP_LOGI(TAG, "Update complete, restarting...");
  esp_restart();

  return ESP_OK;
}
