#pragma once

#include <stdint.h>
#include <string.h>
#include <functional>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_netif.h"

#include "esp_log.h"
#include "mqtt_client.h"


class NixieMQTT final
{
public:
    static NixieMQTT& instance() {
        static NixieMQTT instance;
        return instance;
    }
    NixieMQTT(const NixieMQTT&) = delete;
    NixieMQTT& operator= (const NixieMQTT) = delete;
private:
    NixieMQTT();
    void log_error_if_nonzero(const char *message, int error_code);
public:
    void init(const char *broker);
    inline void setOnCommandHandler(std::function<void(const char *, int)> handler){
        this->onCommandHandler = handler;
    }
    void event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
public:
    
private:
    std::function<void(const char *, int)> onCommandHandler;
    esp_mqtt_client_handle_t client;
    const char *TAG = "NixieMQTT";
    const char *TOPIC_CMD = "esp/nixie/cmd";
    const char *TOPIC_STATUS = "esp/nixie/status";
};