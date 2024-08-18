#include "my_main.h"
pthread_mutex_t lv_mutex;

lv_obj_t *test_anim_obj;
pthread_t thread_app;
void *thread_app_func(void *)
{
    temperature_point_t temperature_point;
    usleep(100000);
    cameraUtils.setColorPalette(IR_COLOR_PALETTE_DEFAULT);
    while (1)
    {
        usleep(25000);
        cameraUtils.getTemperature(&temperature_point);
        pthread_mutex_lock(&lv_mutex);
        lv_obj_set_pos(test_anim_obj, temperature_point.MaxTemperaturePoint.positionX * 320 - 1, temperature_point.MaxTemperaturePoint.positionY * 240 - 1);
        pthread_mutex_unlock(&lv_mutex);
    }
    return NULL;
}
int main()
{
    pthread_mutex_init(&lv_mutex, NULL);
    HAL::init();
    /*
    lv_timer_create([](lv_timer_t *timer)
                    {
        videoPlayer.disconnect();
        lv_timer_del(timer); },
                    30000, NULL);
    */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_sys(), LV_OBJ_FLAG_SCROLLABLE);
    test_anim_obj = lv_obj_create(lv_layer_sys());
    lv_obj_set_style_radius(test_anim_obj, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(test_anim_obj, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(test_anim_obj, 0, 0);
    lv_obj_set_pos(test_anim_obj, 0, 0);
    lv_obj_set_size(test_anim_obj, 3, 3);
    waitboot_scr_load();
    printf("Loop begin\n");
    cameraUtils.initHTTPClient();
    pthread_create(&thread_app, NULL, thread_app_func, NULL);
    HAL::lv_loop();
    return 0;
}