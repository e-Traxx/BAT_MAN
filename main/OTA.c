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

esp_err_t Ota_update(httpd_req_t *req) {
  // Suspend normal funtion
  ESP_LOGI(TAG, "Update Starting");

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

esp_err_t Ota_updater(httpd_req_t *req) {
  // 1. Get the partition where we want to write the new image
  const esp_partition_t *ota_partition =
      esp_ota_get_next_update_partition(NULL);
  if (ota_partition == NULL) {
    ESP_LOGE(TAG, "Failed to find OTA partition");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "No OTA partition found");
    return ESP_FAIL;
  }

  // 2. Begin the OTA process
  esp_ota_handle_t ota_handle = 0;
  esp_err_t err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "OTA begin failed");
    return err;
  }

  // 3. Read the incoming binary data and write it to OTA partition
  char buf[1024];
  int remaining = req->content_len;

  while (remaining > 0) {
    int received = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));
    if (received <= 0) {
      // In a real application, handle this carefully
      ESP_LOGE(TAG, "httpd_req_recv failed: received=%d", received);
      esp_ota_end(ota_handle);
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                          "OTA recv error");
      return ESP_FAIL;
    }
    err = esp_ota_write(ota_handle, buf, received);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "esp_ota_write failed: %s", esp_err_to_name(err));
      esp_ota_end(ota_handle);
      httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                          "OTA write error");
      return err;
    }
    remaining -= received;
  }

  // 4. Complete the OTA update
  err = esp_ota_end(ota_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA end failed");
    return err;
  }

  // 5. Set the OTA partition as the boot partition
  err = esp_ota_set_boot_partition(ota_partition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s",
             esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Set boot partition failed");
    return err;
  }

  // 6. Optionally respond to client that the update was successful
  httpd_resp_sendstr(req, "OTA update successful! Rebooting...\n");

  // 7. Restart
  ESP_LOGI(TAG, "OTA done, restarting...");
  esp_restart();

  // The code below is never reached
  return ESP_OK;
}
