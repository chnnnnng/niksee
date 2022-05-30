#pragma once

#include <stdint.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include "driver/ledc.h"

#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH0_GPIO       hv_en_pin
#define LEDC_LS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_TEST_CH_NUM       (1)

class NixieTube{
private:
    uint8_t digit;
    bool showTimeLock = false;
    gpio_num_t hv_en_pin;
    gpio_num_t sr_data_pin;
    gpio_num_t sr_shift_pin;
    gpio_num_t sr_latch_pin;
    gpio_num_t sr_rst_n_pin;
    int ledc_intr_flag;
private:
    void reset();
    void write_16b(uint16_t data16);
    void gpio_pull_down_once(gpio_num_t);
    void gpio_pull_up_once(gpio_num_t);
    void gpio_pull_high(gpio_num_t);
    void gpio_pull_low(gpio_num_t);
    void pwmInit();
public:
    NixieTube(
        const gpio_num_t hv_en_pin,
        const gpio_num_t data_pin,
        const gpio_num_t shift_pin,
        const gpio_num_t latch_pin,
        const gpio_num_t rst_n_pin
    );
    void enable();  //立即上电
    void disable(); //立即断电
    void show();    //立即显示
    void show(const uint8_t & digit);
    void hide();    //立即熄灭
    void fadeIn();  //渐变显示
    void fadeIn(const uint8_t & digit);
    void fadeOut(); //渐变熄灭
    void detoxificate(int n = 20);  //阴极解毒
    void showTime(const time_t *time);  //显示时间
    void showTime(const tm *time);      //显示时间
    void showIP(const uint8_t ip[4]);   //显示IP地址
    inline void setDigit(const uint8_t & digit){this->digit = digit<=9?digit:0;};
};