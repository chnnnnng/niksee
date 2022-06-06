#include "nixieclock.h"


extern "C" void app_main(void)
{
    NixieInit();
    NixieRun();
}