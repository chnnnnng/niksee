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

class RtcTime{
private:
    tm _time;
    i2c_dev_t _i2c_dev;
    i2c_port_t _i2c_port = 0;
    gpio_num_t _sda;
    gpio_num_t _scl;
    bool _isRunning = false;
public:
    RtcTime(
         const gpio_num_t sda, 
         const gpio_num_t scl
         );
    ~RtcTime();
    bool isRunning();
    tm * get();
    bool set(tm time);
};

class NtpTime{
public:
    void init();
    void obtain();
    inline void setOnObtainedHandler(std::function<void(struct timeval *)> handler){
        this->onObtainedHandler = handler;
    }
private:
    static const char * TAG;
    static std::function<void(struct timeval *)> onObtainedHandler;
};