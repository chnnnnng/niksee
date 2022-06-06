#pragma once

#include <stdint.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "driver/ledc.h"

#define TUBE_ENABLE_FADE    0

#if TUBE_ENABLE_FADE
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH0_GPIO       hv_en_pin_
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_TEST_CH_NUM       (1)
#endif
class NixieTube{
private:
    uint8_t digit_;
    gpio_num_t hv_en_pin_;
    gpio_num_t sr_data_pin_;
    gpio_num_t sr_shift_pin_;
    gpio_num_t sr_latch_pin_;
    gpio_num_t sr_rst_n_pin_;
    int ledc_intr_flag_;
private:
    void Reset();
    void Write(uint16_t data16);
    void GpioPullDownOnce(gpio_num_t);
    void GpioPullUpOnce(gpio_num_t);
    void GpioPullHigh(gpio_num_t);
    void GpioPullLow(gpio_num_t);
    #if TUBE_ENABLE_FADE
    void InitPWM();
    #endif
public:
    NixieTube(
        const gpio_num_t hv_en_pin,
        const gpio_num_t data_pin,
        const gpio_num_t shift_pin,
        const gpio_num_t latch_pin,
        const gpio_num_t rst_n_pin
    );
    void Enable();  //立即上电
    void Disable(); //立即断电
    void Show();    //立即显示
    void Show(const uint8_t & digit);
    void Hide();    //立即熄灭
    #if TUBE_ENABLE_FADE
    void FadeIn();  //渐变显示
    void FadeIn(const uint8_t & digit);
    void FadeOut(); //渐变熄灭
    #endif
    void Detoxificate(int n = 20);  //阴极解毒
    void ShowTime(const time_t *time);  //显示时间
    void ShowTime(const tm *time);      //显示时间
    void ShowIP(const uint8_t ip[4]);   //显示IP地址
    inline void SetDigit(const uint8_t & digit){digit_ = digit<=9?digit:0;};
};