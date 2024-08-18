#include "myHAL.h"
#include "lv_drivers/display/sunxifb.h"
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <linux/input.h>

#define INPUT_DEVICE_ENCODER "/dev/input/event0"
#define INPUT_DEVICE_KEY "/dev/input/event2"

#define IF_CHECK_FILENAME "/proc/net/dev"
#define IF_CHECK_NAME "usb0"
#define IF_CHECK_MAX_SIZE 2048

bool HAL::key_pressed[3] = {0, 0, 0};
time_t HAL::key_pressed_start_time[3] = {0, 0, 0};

static pthread_t thread_key, thread_encoder;
static int encoder_new_move = 0;
static bool encoder_pressed = false;

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
            }
            else
            {
                if (ev_key.value == 1)
                {
                    HAL::key_pressed[ev_key.code - KEY_F1] = true;
                    HAL::key_pressed_start_time[ev_key.code - KEY_F1] = ev_key.time.tv_sec;
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
            encoder_new_move += ev_encoder.value;
        }
    }
    return NULL;
}
// LVGL编码器读取
static void encoder_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    data->enc_diff = encoder_new_move;
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
    pthread_create(&thread_key, NULL, thread_refreshKey, NULL);
    pthread_create(&thread_encoder, NULL, thread_refreshEncoder, NULL);
}
// 初始化HAL
void HAL::init()
{
    static lv_disp_draw_buf_t lv_drawbuf;
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
}
// LVGL定时器处理
void HAL::lv_loop()
{
    while (true)
    {
        pthread_mutex_lock(&lv_mutex);
        lv_task_handler();
        pthread_mutex_unlock(&lv_mutex);
        usleep(2000);
    }
}
