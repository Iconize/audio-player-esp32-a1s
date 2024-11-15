#ifndef WIFI_HPP
#define WIFI_HPP

#include <esp_http_server.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <memory>

#include <esp_wifi.h>
#include <esp_netif.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "lwip/inet.h"
#include "esp_log.h"

#include "esp_websocket_client.h"
#include "esp_http_server.h"

#include "cJSON.h"
#include "http_app.h"
#include "wifi_manager.h"

#include "audio.hpp"

extern httpd_req_t *websocket_req_handle;

namespace Wifi {
    // Tag for logging
    static const char TAG[] = "WIFI";

    // WebSocket handler declaration
    esp_err_t websocket_handler(httpd_req_t *req);

    // Callback when connection is established
    void cb_connection_ok(void *pvParameter);

    // Function to send a WebSocket message
    esp_err_t send_websocket_message(httpd_req_t *req, const char *message);

    // Initialize Wi-Fi and WebSocket setup
    void init();
}

#endif
