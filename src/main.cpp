#include "my_main.h"
pthread_mutex_t lv_mutex;

// lv_obj_t *test_anim_obj;
/// @brief UI线程
pthread_t thread_app;
void *thread_app_func(void *)
{
    temperature_point_t temperature_point;
    while (cameraUtils.connected == false)
        usleep(100000);
    sleep(1);
    cameraUtils.setColorPalette(IR_COLOR_PALETTE_DEFAULT);
    while (1)
    {
        /*
        cameraUtils.getTemperature(&temperature_point);
        LOCKLV();
        lv_obj_set_pos(test_anim_obj, temperature_point.MaxTemperaturePoint.positionX * 320 - 1, temperature_point.MaxTemperaturePoint.positionY * 240 - 1);
        UNLOCKLV();
        */
        usleep(25000);
    }
    return NULL;
}
pthread_t thread_ui;
void *thread_ui_func(void *)
{
    HAL::lv_loop();
}
int main()
{
    sleep(1); // Why?
    system("mkdir " GALLERY_PATH);
    pthread_mutex_init(&lv_mutex, NULL);
    HAL::init();
    readFiles(GALLERY_PATH);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_sys(), LV_OBJ_FLAG_SCROLLABLE);
    // test_anim_obj = lv_obj_create(lv_layer_sys());
    // lv_obj_set_style_radius(test_anim_obj, LV_RADIUS_CIRCLE, 0);
    // lv_obj_set_style_bg_color(test_anim_obj, lv_color_hex(0x00FF00), 0);
    // lv_obj_set_style_border_width(test_anim_obj, 0, 0);
    // lv_obj_set_pos(test_anim_obj, 0, 0);
    // lv_obj_set_size(test_anim_obj, 3, 3);
    printf("Loop begin\n");
    waitboot_scr_load(lv_scr_act());
    pthread_create(&thread_ui, NULL, thread_ui_func, NULL);
    cameraUtils.initHTTPClient();
    pthread_create(&thread_app, NULL, thread_app_func, NULL);
    void *result;
    pthread_join(thread_ui, &result);
    return 0;
}
