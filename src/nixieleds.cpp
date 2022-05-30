#include "nixieleds.h"

SimpleLED::SimpleLED(const gpio_num_t pin)
{
    this->state = false;
    this->pin = pin;
    gpio_set_direction(pin,GPIO_MODE_OUTPUT);
    gpio_set_level(pin,0);
}

void SimpleLED::on()
{
    if(this->state == false) gpio_set_level(pin,1);
    else return;
    this->state = true;
}

void SimpleLED::off()
{
    if(this->state == true) gpio_set_level(pin,0);
    else return;
    this->state = false;
}

void SimpleLED::toggle()
{
    if(this->state == true){
        gpio_set_level(pin,0);
        this->state = false;
    }else{
        gpio_set_level(pin,1);
        this->state = true;
    }
}


