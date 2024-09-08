#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <stdint.h>
#include <stdbool.h>
typedef uint32_t u32;
typedef int32_t s32;
#include <sunxi_display2.h>
#include <sys/ioctl.h>
#include "my_main.h"
static int backlight_fd = 0;
static unsigned long args[4];

int current_backlight = 170;
int target_backlight = 170;
static const uint8_t gamma_lut[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
    4, 4, 5, 5, 5, 5, 6, 6, 6, 7, 7, 7, 8, 8, 8, 9,
    9, 9, 10, 10, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16,
    16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 23, 23, 24, 24,
    25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 32, 33, 34, 35, 35,
    36, 37, 38, 38, 39, 40, 41, 42, 42, 43, 44, 45, 46, 47, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 56, 57, 58, 59, 60, 61, 62, 63,
    64, 65, 66, 67, 68, 69, 70, 71, 73, 74, 75, 76, 77, 78, 79, 80,
    81, 82, 84, 85, 86, 87, 88, 89, 91, 92, 93, 94, 95, 97, 98, 99,
    100, 102, 103, 104, 105, 107, 108, 109, 111, 112, 113, 115, 116, 117, 119, 120,
    121, 123, 124, 126, 127, 128, 130, 131, 133, 134, 136, 137, 139, 140, 142, 143,
    145, 146, 148, 149, 151, 152, 154, 155, 157, 158, 160, 162, 163, 165, 166, 168,
    170, 171, 173, 175, 176, 178, 180, 181, 183, 185, 186, 188, 190, 192, 193, 195,
    197, 199, 200, 202, 204, 206, 207, 209, 211, 213, 215, 217, 218, 220, 222, 224,
    226, 228, 230, 232, 233, 235, 237, 239, 241, 243, 245, 247, 249, 251, 253, 255};
static int Backlight_getCurrent_internal()
{
    int raw;
    args[0] = 0; // 选择fb0(lcd0)
    if (backlight_fd <= 0)
        return 170;
    raw = ioctl(backlight_fd, DISP_LCD_GET_BRIGHTNESS, args);
    for (int i = 0; i < 256; ++i)
    {
        if (raw == gamma_lut[i])
            return i;
    }
    return 255;
}

static void Backlight_setCurrent_internal(int new_backlight)
{
    if (backlight_fd <= 0)
        return;
    if (new_backlight > 255)
        new_backlight = 255;
    args[0] = 0;
    args[1] = gamma_lut[new_backlight];
    args[2] = 0;
    ioctl(backlight_fd, DISP_LCD_SET_BRIGHTNESS, args);
}

#include "pthread.h"

pthread_t thread_backlight;

// 背光UI
void ui_brightnessSlider_create();
void ui_brightnessSlider_loop(int current_brightness);
void *thread_backlight_func(void *param)
{
    (void)param; // unused
    LOCKLV();
    ui_brightnessSlider_create();
    UNLOCKLV();
    while (1)
    {
        if (current_backlight > target_backlight)
        {
            current_backlight -= 1;
            Backlight_setCurrent_internal(current_backlight);
        }
        else if (current_backlight < target_backlight)
        {
            current_backlight += 1;
            Backlight_setCurrent_internal(current_backlight);
        }
        ui_brightnessSlider_loop(current_backlight);
        usleep(15000); // 15 ms
    }
}

void Backlight_set(int new_backlight)
{
    if (new_backlight > 255)
        new_backlight = 255;
    target_backlight = new_backlight;
}

void Backlight_step(int direction)
{
    if (direction > 0)
    {
        if (target_backlight <= (255 - 8))
            target_backlight += 8;
        else
            target_backlight = 255;
    }
    else
    {
        if (target_backlight >= 8)
            target_backlight -= 8;
        else
            target_backlight = 0;
    }
}

void Backlight_init()
{
    backlight_fd = open("/dev/disp", O_RDWR, 0);
    if (backlight_fd <= 0)
    {
        printf("open /dev/disp failed.\n");
        return;
    }
    target_backlight = Backlight_getCurrent_internal();
    current_backlight = target_backlight;
    pthread_create(&thread_backlight, NULL, thread_backlight_func, NULL);
    printf("brightness init finished\n");
}