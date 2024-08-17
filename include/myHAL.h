#pragma once
#include "my_main.h"
namespace HAL
{
    extern bool key_pressed[3];
    extern time_t key_pressed_start_time[3];
    void init();
    void lv_loop();
    bool checkCameraIF();
    void setCameraIFIP();
} // namespace HAL
