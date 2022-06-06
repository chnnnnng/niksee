#pragma once

#include <iostream>
#include <ds1307.h>
#include <memory.h>
#include <stdint.h>
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
#include "esp_sntp.h"

class RtcTime final
{
public:
    static RtcTime& instance(const gpio_num_t sda = GPIO_NUM_NC, const gpio_num_t scl = GPIO_NUM_NC) {
        static RtcTime instance(sda,scl);
        return instance;
    }
    RtcTime(const RtcTime&) = delete;
    RtcTime& operator= (const RtcTime) = delete;
private:
    RtcTime(const gpio_num_t sda, const gpio_num_t scl);
public:
    void Deinit(void);
    tm * Get();
    bool Set(tm time);
    bool IsRunning();
private:
    const char *TAG_ = "RtcTime";
    esp_err_t err_;
    tm time_;
    i2c_dev_t i2c_dev_;
    i2c_port_t i2c_port_ = 0;
    gpio_num_t sda_;
    gpio_num_t scl_;
    bool is_running_ = false;
};


class NtpTime{
public:
    static NtpTime& instance() {
        static NtpTime instance;
        return instance;
    }
    NtpTime(const NtpTime&) = delete;
    NtpTime& operator= (const NtpTime) = delete;
private:
    NtpTime();
public:
    void Init();
    void Obtain();
    inline void SetOnObtainedHandler(std::function<void(struct timeval *)> handler){
        this->on_obtain_handler_ = handler;
    }
private:
    const char * TAG_ = "NtpTime";
    std::function<void(struct timeval *)> on_obtain_handler_;
};