#pragma once
// 参数配置
#define VIDEO_STREAM_URL "rtsp://admin:Ab123456@192.168.64.64"
#define GALLERY_PATH "/mnt/UDISK/DCIM" // 如需修改存储位置，需同时修改 lv_conf.h: LV_FS_STDIO_PATH
// 注意：图像命名格式固定为：CAP+5位数字.[jpeg/mjpeg]
#define SETTINGS_PATH "/mnt/UDISK/settings.dat"
// 系统头文件
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>
#include <lvgl.h>
// 用户头文件
#include "myHAL.h"
#include "videoPlayer.h"
#include "videoCodec.h"
#include "animations.h"
#include "cameraUtils.h"
#include "utils/filenameAllocator.h"
#include "ui.h"
#include "settingsStorage.h"                // 设置存储
// 字体
#ifdef __cplusplus
extern "C"
{
#endif
    extern const lv_font_t ui_font_ali24;
    extern const lv_font_t ui_font_chinese16;
    extern const lv_font_t ui_font_fontawesome16;
    extern const lv_font_t ui_font_fontawesome24;
#ifdef __cplusplus
}
#endif
// 全局变量
extern pthread_mutex_t lv_mutex;
extern bool global_poweroff_request; // main.cpp

// lvgl互斥锁
#define LOCKLV() pthread_mutex_lock(&lv_mutex)
#define UNLOCKLV() pthread_mutex_unlock(&lv_mutex)

// 等待动画
void waitboot_scr_load(lv_obj_t *parent);

// 电源管理相关
int16_t PowerManager_getBatteryVoltage();
bool PowerManager_isCharging();
void PowerManager_init();
void PowerManager_powerOff();

// 背光模块
void Backlight_init();
void Backlight_set(int new_backlight);
void Backlight_step(int direction);

/// @brief 相册事件检查
/// @param has_hal_go_back_event 如果为true，代表返回（菜单）按键按下
void menu_gallery_loop(bool has_hal_go_back_event);

// 图表功能
void widget_graph_create();
void widget_graph_updateSettings();
void widget_graph_check_visibility();

void ui_crosshairs_create();
void ui_crosshairs_updatePos();
void ui_crosshairs_updateVisibility();