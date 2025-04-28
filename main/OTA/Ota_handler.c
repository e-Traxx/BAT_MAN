#include "OTA.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "esp_flash.h"
#include <string.h>
#include <inttypes.h>

static const char *TAG = "OTA";
#define MIN(x, y) ((x) < (y) ? (x) : (y))

esp_err_t Ota_updater(httpd_req_t *req) {
  // 1. Verify partition table
  const esp_partition_t *running = esp_ota_get_running_partition();
  if (running == NULL) {
    ESP_LOGE(TAG, "Failed to get running partition");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Failed to get running partition");
    return ESP_FAIL;
  }

  // 2. Get the partition where we want to write the new image
  const esp_partition_t *ota_partition =
      esp_ota_get_next_update_partition(NULL);
  if (ota_partition == NULL) {
    ESP_LOGE(TAG, "Failed to find OTA partition");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "No OTA partition found");
    return ESP_FAIL;
  }

  // 3. Verify partition size
  if (ota_partition->size < req->content_len) {
    ESP_LOGE(TAG, "OTA partition too small");
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "OTA partition too small");
    return ESP_FAIL;
  }

  // 4. Begin the OTA process
  esp_ota_handle_t ota_handle = 0;
  esp_err_t err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ota_begin failed: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "OTA begin failed");
    return err;
  }

  // 5. Read the incoming binary data and write it to OTA partition
  char buf[1024];
  int remaining = req->content_len;
  size_t total_written = 0;

  while (remaining > 0) {
    int received = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));
    if (received <= 0) {
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

    total_written += received;
    remaining -= received;
  }

  // 6. Verify the written data size
  if (total_written != req->content_len) {
    ESP_LOGE(TAG, "OTA update incomplete: %d != %d", total_written, req->content_len);
    esp_ota_end(ota_handle);
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "OTA update incomplete");
    return ESP_FAIL;
  }

  // 7. Complete the OTA update
  err = esp_ota_end(ota_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ota_end failed: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "OTA end failed");
    return err;
  }

  // 8. Verify the new firmware
  err = esp_ota_check_rollback_is_possible();
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Rollback check failed: %s", esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Firmware verification failed");
    return err;
  }

  // 9. Set the OTA partition as the boot partition
  err = esp_ota_set_boot_partition(ota_partition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "esp_ota_set_boot_partition failed: %s",
             esp_err_to_name(err));
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR,
                        "Set boot partition failed");
    return err;
  }

  // 10. Respond to client that the update was successful
  httpd_resp_sendstr(req, "OTA update successful! Rebooting...\n");

  // 11. Restart
  ESP_LOGI(TAG, "OTA done, restarting...");
  esp_restart();

  // The code below is never reached
  return ESP_OK;
}
