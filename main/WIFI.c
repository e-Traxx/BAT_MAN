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

void WIFI_Setup() {
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  wifi_config_t ap_config = {.ap = {.ssid = "BAT-MAN",
                                    .ssid_len = strlen("BAT-MAN"),
                                    .password = "IamBATmanExxe25",
                                    .max_connection = 4,
                                    .authmode = WIFI_AUTH_WPA2_PSK

                             }};
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI("WIFI", "AP started. SSID: BAT-MAN, password: IamBATmanExxe25");

  // Start HTTP Server
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;
  httpd_start(&server, &config);

  httpd_uri_t ota_uri = {.uri = "/update",
                         .method = HTTP_POST,
                         .handler = Ota_updater,
                         .user_ctx = NULL};
  httpd_register_uri_handler(server, &ota_uri);
}
