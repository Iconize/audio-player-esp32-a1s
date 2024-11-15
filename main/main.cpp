#include "audio.hpp"
#include "wifi.hpp"

extern "C" void app_main(void)
{
    Wifi::init();
    Audio::init();
}
