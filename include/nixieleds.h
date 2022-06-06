#pragma once

#include <driver/gpio.h>

class SimpleLED{
private:
    bool state_;
    gpio_num_t pin_;
public:
    SimpleLED(const gpio_num_t pin);
    void On();
    void Off();
    void Toggle();
};