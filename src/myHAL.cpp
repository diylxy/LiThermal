#include "myHAL.h"
#include "lv_drivers/display/sunxifb.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <linux/input.h>

void refresh_poweroff_key(); // 处理关机按钮（左侧第一个按钮），位于poweroff.cpp
void refresh_menu_key();     // 左侧第二个按钮，button_2.cpp
void battery_card_check();   // 电池电量刷新，batteryCard.cpp

#define INPUT_DEVICE_ENCODER "/dev/input/event0"
#define INPUT_DEVICE_KEY "/dev/input/event2"

#define IF_CHECK_FILENAME "/proc/net/dev"
#define IF_CHECK_NAME "usb0"
#define IF_CHECK_MAX_SIZE 2048

bool HAL::key_pressed[3] = {0, 0, 0};
time_t HAL::key_pressed_start_time[3] = {0, 0, 0};
bool HAL::key_press_event[4] = {0, 0, 0};

int last_encoder_direction = 0; // 记录上次滚动方向，用于在主界面跟踪编码器
current_mode_t current_mode = MODE_MAINPAGE;
bool global_poweroff_request = false;

static pthread_t thread_key, thread_encoder;
static int encoder_new_move = 0;
static bool encoder_pressed = false;
static pthread_t thread_hal;
// Ctrl+C退出
void signal_exit(int signo)
{
#ifdef CMAKE_CROSSCOMPILING
    sunxifb_exit();
    printf("exit\n");
#endif
    videoPlayer.disconnect();
    sleep(1);
    exit(0);
}
// 按键刷新线程
static void *thread_refreshKey(void *)
{
    int fd_key = open(INPUT_DEVICE_KEY, O_RDONLY);
    if (fd_key < 0)
    {
        perror("cannot open keys");
        return NULL;
    }
    struct input_event ev_key;
    while (1)
    {
        read(fd_key, &ev_key, sizeof(struct input_event));
        if (ev_key.type == EV_KEY)
        {
            if (ev_key.code == KEY_ENTER)
            {
                encoder_pressed = ev_key.value == 1;
                if (encoder_pressed)
                    HAL::key_press_event[3] = true;
            }
            else
            {
                if (ev_key.value == 1)
                {
                    HAL::key_pressed[ev_key.code - KEY_F1] = true;
                    HAL::key_pressed_start_time[ev_key.code - KEY_F1] = ev_key.time.tv_sec;
                    HAL::key_press_event[ev_key.code - KEY_F1] = true;
                }
                else
                {
                    HAL::key_pressed[ev_key.code - KEY_F1] = false;
                }
            }
        }
    }
    return NULL;
}
// 编码器刷新线程
static void *thread_refreshEncoder(void *)
{
    int fd_encoder = open(INPUT_DEVICE_ENCODER, O_RDONLY);
    if (fd_encoder < 0)
    {
        perror("cannot open encoder");
        return NULL;
    }
    struct input_event ev_encoder;
    while (1)
    {
        read(fd_encoder, &ev_encoder, sizeof(struct input_event));
        if (ev_encoder.type == EV_REL)
        {
            encoder_new_move -= ev_encoder.value; // 反转编码器方向
        }
    }
    return NULL;
}
// LVGL编码器读取
static void encoder_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (current_mode != MODE_MAINPAGE && current_mode != MODE_GALLERY)
        data->enc_diff = encoder_new_move;
    else
        data->enc_diff = 0;
    if (encoder_new_move > 0)
    {
        last_encoder_direction = 1;
    }
    else if (encoder_new_move < 0)
    {
        last_encoder_direction = -1;
    }
    encoder_new_move = 0;

    if (encoder_pressed)
        data->state = LV_INDEV_STATE_PRESSED;
    else
        data->state = LV_INDEV_STATE_RELEASED;
}
// 初始化输入设备
static inline void initInputDevices()
{
    /*Initialize and register a input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb = encoder_read;
    /*Register the driver in LVGL and save the created input device object*/
    lv_indev_t *my_indev = lv_indev_drv_register(&indev_drv);
    lv_group_t *group = lv_group_create();
    lv_group_set_default(group);
    lv_indev_set_group(my_indev, group);
    pthread_create(&thread_key, NULL, thread_refreshKey, NULL);
    pthread_create(&thread_encoder, NULL, thread_refreshEncoder, NULL);
}
/// @brief hal线程，如按钮状态刷新
void *thread_hal_func(void *)
{
    while (1)
    {
        refresh_poweroff_key();
        if (global_poweroff_request == true)
        {
            global_poweroff_request = false;
            usleep(500 * 1000);
            PowerManager_powerOff();
        }

        if (current_mode == MODE_MAINPAGE)
        {
            if (last_encoder_direction != 0)
            {
                Backlight_step(last_encoder_direction);
                last_encoder_direction = 0;
            }
            if (HAL::key_press_event[2])
            {
                // 开始/停止录像
                HAL::key_press_event[2] = false;
            }
            if (HAL::key_press_event[3])
            {
                camera_take_photo_from_stream();
                HAL::key_press_event[3] = false;
            }
        }
        else if (current_mode == MODE_GALLERY || current_mode == MODE_GALLERY_MENU)
        {
            // 处理相册操作
            menu_gallery_loop(false); // false表示没有返回事件
        }
        else // 清零提取出的编码器变化值
        {
            last_encoder_direction = 0;
            HAL::key_press_event[2] = false;
            HAL::key_press_event[3] = false;
        }
        refresh_menu_key();
        battery_card_check();
        usleep(25000);
    }
}
// 初始化HAL
void HAL::init()
{
    static lv_disp_draw_buf_t lv_drawbuf;
    // 硬件初始化
    PowerManager_init();
    // 软件初始化
    lv_init();
    printf("Hello World\n");
    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    static lv_color_t *buf;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
    disp_drv.draw_buf = &lv_drawbuf;
    printf("Running On Device\n");
    uint32_t rotated = LV_DISP_ROT_NONE;

    /*Linux frame buffer device init*/
    sunxifb_init(rotated);

    /*A buffer for LittlevGL to draw the screen's content*/
    static uint32_t width, height;
    sunxifb_get_sizes(&width, &height);

    buf = (lv_color_t *)sunxifb_alloc(width * height * sizeof(lv_color_t),
                                      const_cast<char *>("lv_examples"));

    if (buf == NULL)
    {
        sunxifb_exit();
        printf("malloc draw buffer fail\n");
        return;
    }
    disp_drv.flush_cb = sunxifb_flush;
    disp_drv.hor_res = width;
    disp_drv.ver_res = height;
    disp_drv.rotated = rotated;
    lv_disp_drv_register(&disp_drv);
    initInputDevices();
    lv_disp_draw_buf_init(&lv_drawbuf, buf, NULL, 320 * 240 * 4);
    signal(SIGINT, signal_exit);
    // HAL 线程
    pthread_create(&thread_hal, NULL, thread_hal_func, NULL);
    // 背光初始化
    Backlight_init();
}
// LVGL定时器处理
void HAL::lv_loop()
{
    while (true)
    {
        LOCKLV();
        lv_task_handler();
        UNLOCKLV();
        usleep(5000);
    }
}
