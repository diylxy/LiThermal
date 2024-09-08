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

typedef enum {
    MODE_MAINPAGE,
    MODE_MAINMENU,
    MODE_GALLERY,
    MODE_GALLERY_MENU,
    MODE_CAMERA_SETTINGS,
    MODE_SYSTEM_SETTINGS,
}current_mode_t;
extern current_mode_t current_mode;         // 当前设备所处模式
extern int last_encoder_direction;          // 记录编码器变化，用于亮度调节或相册
