#include "OTA.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_https_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "esp_wifi_types_generic.h"
#include "lwip/ip4_addr.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "WIFI";

void WIFI_Setup() {
  // 1. Initialize NVS (required by Wi-Fi)
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // 2. Initialize the TCP/IP stack
  ESP_ERROR_CHECK(esp_netif_init());

  // 3. Create default event loop
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  // 4. Create default Wi-Fi AP netif (internally registers it to the event
  // loop)
  esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();

  // 5. Initialize Wi-Fi with default config
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  // 6. Configure and Start Wi-Fi in AP mode
  wifi_config_t wifi_config = {
      .ap = {.ssid = "BATMAN",
             .ssid_len = strlen("BATMAN"),
             .password = "IamBATmanExxe25",
             .max_connection = 5,
             .authmode = WIFI_AUTH_OPEN},
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));

  // 7. Stop DHCP server on the AP interface so we can set static IP
  ESP_ERROR_CHECK(esp_netif_dhcps_stop(ap_netif));

  // 8. Set static IP information
  esp_netif_ip_info_t ip_info;
  IP4_ADDR(&ip_info.ip, 192, 168, 5, 1);
  IP4_ADDR(&ip_info.gw, 192, 168, 5, 1);
  IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
  ESP_ERROR_CHECK(esp_netif_set_ip_info(ap_netif, &ip_info));

  // 9. (Re)start DHCP server if you actually want to serve IP addresses
  ESP_ERROR_CHECK(esp_netif_dhcps_start(ap_netif));
  ESP_LOGI(TAG, "Static AP IP set to: " IPSTR, IP2STR(&ip_info.ip));

  // 10. Finally start Wi-Fi
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Wi-Fi AP started.");

  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;

  ESP_ERROR_CHECK(httpd_start(&server, &config));

  httpd_uri_t ota_uri = {.uri = "/update",
                         .method = HTTP_POST,
                         .handler = Ota_updater,
                         .user_ctx = NULL};
  ESP_ERROR_CHECK(httpd_register_uri_handler(server, &ota_uri));
}
