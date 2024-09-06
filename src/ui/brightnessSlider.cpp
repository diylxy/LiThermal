#include "my_main.h"

// 主界面滚动滚轮为调节亮度
// 跳过lvgl焦点策略，直接在hal线程调节，并弹出亮度条实时反馈
static lv_obj_t *ui_brightnessSlider = NULL;
#define UI_BRIGHTNESSBAR_Y -60
#define UI_BRIGHTNESSBAR_X_HIDE 50
#define UI_BRIGHTNESSBAR_X_SHOW -10
#define UI_BRIGHTNESSBAR_HEIGHT_HIDE 60
#define UI_BRIGHTNESSBAR_HEIGHT_SHOW 102
#define BRIGHTNESSBAR_AUTO_HIDE_TIME 1 // auto hide when inactive for 2s

void ui_brightnessSlider_create()
{
    if (ui_brightnessSlider != NULL)
        return;

    ui_brightnessSlider = lv_bar_create(lv_layer_sys());
    lv_bar_set_range(ui_brightnessSlider, 0, 255);
    lv_bar_set_value(ui_brightnessSlider, 75, LV_ANIM_OFF);
    lv_bar_set_start_value(ui_brightnessSlider, 0, LV_ANIM_OFF);
    lv_obj_set_width(ui_brightnessSlider, 40);
    lv_obj_set_height(ui_brightnessSlider, UI_BRIGHTNESSBAR_HEIGHT_SHOW);
    lv_obj_set_x(ui_brightnessSlider, UI_BRIGHTNESSBAR_X_HIDE);
    lv_obj_set_y(ui_brightnessSlider, UI_BRIGHTNESSBAR_Y);
    lv_obj_set_align(ui_brightnessSlider, LV_ALIGN_RIGHT_MID);
    lv_obj_set_style_radius(ui_brightnessSlider, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_brightnessSlider, lv_color_hex(0xFFE99A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_brightnessSlider, 80, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_set_style_radius(ui_brightnessSlider, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_brightnessSlider, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_brightnessSlider, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);

    lv_obj_t *ui_Label4 = lv_label_create(ui_brightnessSlider);
    lv_obj_set_x(ui_Label4, 0);
    lv_obj_set_y(ui_Label4, -8);
    lv_obj_set_align(ui_Label4, LV_ALIGN_BOTTOM_MID);
    lv_label_set_text(ui_Label4, "");
    lv_obj_set_style_text_color(ui_Label4, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_opa(ui_Label4, 200, 0);
    lv_obj_set_style_text_font(ui_Label4, &ui_font_fontawesome16, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void ui_brightnessSlider_loop(int current_brightness)
{
    static int last_brightness = -1;
    static time_t last_tick = -1;
    static bool expanded = false;
    if (last_brightness == -1)
    {
        // 第一次循环
        last_tick = time(NULL);
        last_brightness = current_brightness;
    }
    if (last_brightness != current_brightness)
    {
        if (expanded == false)
        {
            expanded = true;
            LOCKLV();
            lv_anim_move(ui_brightnessSlider, UI_BRIGHTNESSBAR_X_SHOW, UI_BRIGHTNESSBAR_Y);
            lv_anim_size(ui_brightnessSlider, 40, UI_BRIGHTNESSBAR_HEIGHT_SHOW);
            UNLOCKLV();
        }
        last_tick = time(NULL);
        LOCKLV();
        lv_bar_set_value(ui_brightnessSlider, current_brightness, LV_ANIM_OFF);
        UNLOCKLV();
        last_brightness = current_brightness;
    }
    else
    {
        if (expanded == true)
        {
            if (time(NULL) - last_tick >= BRIGHTNESSBAR_AUTO_HIDE_TIME)
            {
                expanded = false;
                LOCKLV();
                lv_anim_move(ui_brightnessSlider, UI_BRIGHTNESSBAR_X_HIDE, UI_BRIGHTNESSBAR_Y);
                lv_anim_size(ui_brightnessSlider, 40, UI_BRIGHTNESSBAR_HEIGHT_HIDE);
                UNLOCKLV();
            }
        }
    }
}
