#pragma once

#include <driver/gpio.h>

class SimpleLED{
private:
    bool state;
    gpio_num_t pin;
public:
    SimpleLED(const gpio_num_t pin);
    void on();
    void off();
    void toggle();
};