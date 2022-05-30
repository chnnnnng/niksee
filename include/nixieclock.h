#pragma once

#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

#include "nixietube.h"
#include "nixieleds.h"
#include "nixietime.h"
#include "nixienetwork.h"
#include "nixiestorage.h"
#include "nixiemqtt.h"



class NixieClock final{
private:
    static NixieTube tube;
    static SimpleLED onboard_led;
    static RtcTime rtc;
    static NtpTime ntp;

    static TaskHandle_t nixieShowCurrentTimeTaskHandler;
    static TaskHandle_t nixieConfigIndicationTaskHandler;
//public interfaces
public: 
    static void init();
    static void run();
//initializers

//tasks
private:
    static void nixieShowCurrentTimeTask(void * pvParameters);
    static void nixieConfigIndicationTask(void *);
//eventHandlers

};