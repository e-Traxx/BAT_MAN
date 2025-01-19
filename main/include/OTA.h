#ifndef OTA_H
#define OTA_H

#include "esp_err.h"
#include "esp_http_server.h"

esp_err_t Ota_updater(httpd_req_t *req);

#endif
