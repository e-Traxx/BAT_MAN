# idf_component_register(SRCS "flash_storage.c" "Batman_esp.c" "can_handler.c" "diagnostic_handler.c" "spi_handler.c" "Robin" "OTA" "WIFI.c" "Albert" "Diag"
#                     INCLUDE_DIRS "include")

idf_component_register(
    SRCS 
        "Batman_esp.c"
        "Oracle/Oracle.c"
        "Robin_spi/Robin_parser.c"
        # "Robin/Robin_handler.c"
        "Albert/Albert.c"
        "OTA/Ota_handler.c"
        "WIFI.c"
        "can_handler.c"
        "spi_handler.c"
        "diagnostic_handler.c"
        "isospi_handler.c"
        "uart_handler.c"
        "Robin_TI/Robin_handler_TI.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES 
        driver
        esp_netif
        esp_wifi
        nvs_flash
        esp_http_server
        json
        esp_http_client
        app_update
        esp_adc
        esp_driver_gpio
        esp_driver_spi
        spi_flash
)
