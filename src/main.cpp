#include "my_main.h"
pthread_mutex_t lv_mutex;

lv_obj_t *test_anim_obj;
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
    waitboot_scr_load();
    printf("Loop begin\n");
    HAL::lv_loop();
    return 0;
}