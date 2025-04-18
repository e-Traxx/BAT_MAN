#include "OTA.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_ota_ops.h"
#include "esp_wifi.h"
#include "lwip/ip4_addr.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "WIFI";
static esp_netif_t *ap_netif = NULL;
static httpd_handle_t server = NULL;

static esp_err_t wifi_init_nvs(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "NVS partition erased and will be initialized");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

static void wifi_cleanup(void) {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
    
    if (ap_netif) {
        esp_netif_dhcps_stop(ap_netif);
        esp_netif_destroy(ap_netif);
        ap_netif = NULL;
    }
    
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_loop_delete_default();
    esp_netif_deinit();
    nvs_flash_deinit();
}

esp_err_t WIFI_Setup() {
    esp_err_t ret = ESP_OK;

    // 1. Initialize NVS
    ret = wifi_init_nvs();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }

    // 2. Initialize the TCP/IP stack
    ret = esp_netif_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize TCP/IP stack: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    // 3. Create default event loop
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create event loop: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    // 4. Create default Wi-Fi AP netif
    ap_netif = esp_netif_create_default_wifi_ap();
    if (!ap_netif) {
        ESP_LOGE(TAG, "Failed to create default AP netif");
        wifi_cleanup();
        return ESP_FAIL;
    }

    // 5. Initialize Wi-Fi with default config
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Wi-Fi: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    // 6. Configure and Start Wi-Fi in AP mode
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "BATMAN",
            .ssid_len = strlen("BATMAN"),
            .password = "IamBATmanExxe25",
            .max_connection = 5,
            .authmode = WIFI_AUTH_OPEN
        },
    };

    ret = esp_wifi_set_mode(WIFI_MODE_AP);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set Wi-Fi mode: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    ret = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set Wi-Fi config: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    // 7. Stop DHCP server
    ret = esp_netif_dhcps_stop(ap_netif);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop DHCP server: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    // 8. Set static IP information
    esp_netif_ip_info_t ip_info;
    IP4_ADDR(&ip_info.ip, 192, 168, 5, 1);
    IP4_ADDR(&ip_info.gw, 192, 168, 5, 1);
    IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
    
    ret = esp_netif_set_ip_info(ap_netif, &ip_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set IP info: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    // 9. Start DHCP server
    ret = esp_netif_dhcps_start(ap_netif);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start DHCP server: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }
    ESP_LOGI(TAG, "Static AP IP set to: " IPSTR, IP2STR(&ip_info.ip));

    // 10. Start Wi-Fi
    ret = esp_wifi_start();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Wi-Fi: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    ESP_LOGI(TAG, "Wi-Fi AP started.");

    // 11. Start HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ret = httpd_start(&server, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    // 12. Register OTA endpoint
    httpd_uri_t ota_uri = {
        .uri = "/update",
        .method = HTTP_POST,
        .handler = Ota_updater,
        .user_ctx = NULL
    };
    
    ret = httpd_register_uri_handler(server, &ota_uri);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register OTA URI handler: %s", esp_err_to_name(ret));
        wifi_cleanup();
        return ret;
    }

    return ESP_OK;
}

void WIFI_Cleanup(void) {
    wifi_cleanup();
}
