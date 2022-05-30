#pragma once

#include <iostream>
#include <stdint.h>
#include <memory.h>
#include <functional>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_netif.h"
#include "esp_smartconfig.h"

#include "nixiestorage.h"


class NixieNetwork final
{
public:
    static NixieNetwork& instance() {
        static NixieNetwork instance;
        return instance;
    }
    NixieNetwork(const NixieNetwork&) = delete;
    NixieNetwork& operator= (const NixieNetwork) = delete;
private:
    NixieNetwork();
public://public properties

public://public methods
    void init();
    inline void setOnIpGotHandler(std::function<void(esp_ip4_addr_t)> handler){
        onIpGotHandler = handler;
    }
private://private properties
    EventGroupHandle_t s_wifi_event_group;
    const int CONNECTED_BIT = BIT0;
    const int ESPTOUCH_DONE_BIT = BIT1;
    const char * TAG = "NixieNetwork";
    TimerHandle_t auto_close_timer;
    wifi_config_t wifi_config_stored;
    bool hasStoredConfig = false;
    esp_err_t err;
    esp_netif_t *sta_netif;
    std::function<void(esp_ip4_addr_t)> onIpGotHandler;
private://private methods
    void networkHandler(
        void* arg,
        esp_event_base_t event_base,
        int32_t event_id,
        void* event_data);
    void sc_auto_close_callback( TimerHandle_t xTimer);
    void networkSmartConfigTask(void * pvParameters);
    bool readStoredWifiConfig(uint8_t ssid[32],uint8_t password[64]);
    bool saveStoredWifiConfig(uint8_t ssid[32],uint8_t password[64]);
    void createAutoCloseTimer();
};


// class NixieNetwork final{
// private:
//     static EventGroupHandle_t s_wifi_event_group;
//     static const int CONNECTED_BIT;
//     static const int ESPTOUCH_DONE_BIT;
//     static const char *TAG;
//     static TimerHandle_t auto_close_timer;
//     static wifi_config_t wifi_config_stored;
//     static bool hasStoredConfig;
// public:
//     static void initNetwork();
// private:
//     static void networkHandler(
//         void* arg,
//         esp_event_base_t event_base,
//         int32_t event_id,
//         void* event_data);
//     static void sc_auto_close_callback( TimerHandle_t xTimer);
//     static void networkSmartConfigTask(void * pvParameters);
//     static bool readStoredWifiConfig(uint8_t ssid[32],uint8_t password[64]);
//     static bool saveStoredWifiConfig(uint8_t ssid[32],uint8_t password[64]);
// };