#include "nixieleds.h"

SimpleLED::SimpleLED(const gpio_num_t pin):state_(false),pin_(pin)
{
    gpio_set_direction(pin,GPIO_MODE_OUTPUT);
    gpio_set_level(pin,0);
}

void SimpleLED::On()
{
    if(state_ == false) gpio_set_level(pin_,1);
    else return;
    state_ = true;
}

void SimpleLED::Off()
{
    if(state_ == true) gpio_set_level(pin_,0);
    else return;
    state_ = false;
}

void SimpleLED::Toggle()
{
    if(state_){
        gpio_set_level(pin_,0);
    }else{
        gpio_set_level(pin_,1);
    }
    state_ = !state_;
}


