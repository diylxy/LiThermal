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
#include "videoPlayer.h"
#include "animations.h"
#include "cameraUtils.h"

void waitboot_scr_load(void);
extern pthread_mutex_t lv_mutex;