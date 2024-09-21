#include "my_main.h"
pthread_mutex_t lv_mutex;

// lv_obj_t *test_anim_obj;
/// @brief 热成像刷新线程
pthread_t thread_app;
void *thread_app_func(void *)
{
    static uint32_t last_color_palette = -1;
    static uint32_t last_show_center = -1;
    static int centerRefreshCounter = 0;
    while (cameraUtils.connected == false)
        usleep(100000);
    sleep(1);
    LOCKLV();
    widget_graph_updateSettings();
    ui_crosshairs_updateVisibility();
    widget_graph_check_visibility();
    UNLOCKLV();
    while (1)
    {
        if (last_color_palette != globalSettings.colorPalette)
        {
            last_color_palette = globalSettings.colorPalette;
            cameraUtils.setColorPalette(globalSettings.colorPalette);
        }
        if (last_show_center != globalSettings.enableCenterValueDisplay)
        {
            last_show_center = globalSettings.enableCenterValueDisplay;
            cameraUtils.setCenterMeasure(last_show_center);
        }
        if (current_mode == MODE_MAINPAGE || current_mode == MODE_CAMERA_SETTINGS)
        {
            cameraUtils.getTemperature();
        }
        LOCKLV();
        ui_crosshairs_updatePos();
        widget_graph_check_visibility();
        UNLOCKLV();
        usleep(40000);
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
    widget_graph_create();
    ui_crosshairs_create();
    pthread_create(&thread_ui, NULL, thread_ui_func, NULL);
    cameraUtils.initHTTPClient();
    pthread_create(&thread_app, NULL, thread_app_func, NULL);
    void *result;
    pthread_join(thread_ui, &result);
    return 0;
}
