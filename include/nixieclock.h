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


/**
 * global variables declaration
 */
extern NixieTube tube;
extern SimpleLED onboard_led;
extern RtcTime& rtc;
extern NtpTime& ntp;
extern NixieStorage& storage;
extern NixieNetwork& network;
extern NixieMQTT& mqtt;


/**
 * variables declaration
 */


/**
 * global functions declaration
 */
extern void NixieInit(void);
extern void NixieRun(void);


/**
 * functions declaration
 */
void NixieShowTimeTask(void *);
void NixieShowConfigIndicationTask(void *);