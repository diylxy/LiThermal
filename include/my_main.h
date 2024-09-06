#pragma once
#define CMAKE_CROSSCOMPILING
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
#ifndef CMAKE_CROSSCOMPILING
#include <sdl/sdl.h>
#endif

#define VIDEO_STREAM_URL "rtsp://admin:Ab123456@192.168.64.64"

#include "myHAL.h"
extern bool global_poweroff_request;       // main.cpp
#include "videoPlayer.h"
#include "animations.h"
#include "cameraUtils.h"

extern pthread_mutex_t lv_mutex;
#define LOCKLV() pthread_mutex_lock(&lv_mutex)
#define UNLOCKLV() pthread_mutex_unlock(&lv_mutex)

// 等待动画
#ifdef __cplusplus
extern "C" {
#endif
extern const lv_font_t ui_font_chinese16;
extern const lv_font_t ui_font_fontawesome16;
extern const lv_font_t ui_font_fontawesome24;
#ifdef __cplusplus
}
#endif
void waitboot_scr_load(lv_obj_t *parent);

// 电源管理相关
int16_t PowerManager_getBatteryVoltage();
void PowerManager_init();
void PowerManager_powerOff();

// 背光模块
void Backlight_init();
void Backlight_set(int new_backlight);
void Backlight_step(int direction);

// 背光UI
void ui_brightnessSlider_create();
void ui_brightnessSlider_loop(int current_brightness);
