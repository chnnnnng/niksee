#include "nixieclock.h"

extern "C" void app_main(void)
{
    NixieClock::init();
    NixieClock::run();  
}