#pragma once
#include "my_main.h"
namespace HAL
{
    extern bool key_pressed[3];
    extern time_t key_pressed_start_time[3]; // 注意不包括Enter
    extern bool key_press_event[4];          //  包括Enter
    void init();
    void lv_loop();
} // namespace HAL

extern bool trap_encoder; // 标记当前不在菜单里，此时编码器用于调节亮度
#define EXIT_MAINPAGE() trap_encoder = false
#define BACK_TO_MAINPAGE() trap_encoder = true
#define IN_MAINPAGE (trap_encoder == true)
#define NOT_IN_MAINPAGE (trap_encoder == false)