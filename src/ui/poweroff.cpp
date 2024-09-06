#include "my_main.h"

static void doPoweroff()
{
    printf("Poweroff!\n");
    global_poweroff_request = true;// 此处通知HAL线程关机，然后返回，在此注册背光渐暗动画。主线程等待动画完全结束（500ms 左右）再启动关机流程
}
bool poweroff_started = true;
static MyCard mycardPower;
static lv_obj_t *blackOverlay = NULL;
static int32_t animation_opa = 0;
static lv_obj_t *lbl_power_off_prompt = NULL;
static void lv_ui_poweroff()
{
    if (blackOverlay != NULL)
        return;
    LOCKLV();
    blackOverlay = lv_obj_create(lv_layer_sys());
    mycardPower.create(lv_layer_sys());
    lv_obj_set_style_border_width(blackOverlay, 0, 0);
    lv_obj_set_style_radius(blackOverlay, 0, 0);
    lv_obj_set_size(blackOverlay, 320, 240);
    lv_obj_set_style_opa(blackOverlay, 0, 0);
    lv_obj_set_style_bg_color(blackOverlay, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_color(mycardPower.obj, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(mycardPower.obj, LV_OBJ_FLAG_SCROLLABLE);
    if (lbl_power_off_prompt == NULL || (lv_obj_is_valid(lbl_power_off_prompt) == false))
    {
        lbl_power_off_prompt = lv_label_create(mycardPower.obj);
        lv_obj_set_style_text_font(lbl_power_off_prompt, &ui_font_chinese16, 0);
        lv_obj_center(lbl_power_off_prompt);
    }
    mycardPower.show(CARD_ANIM_NONE);
    mycardPower.move(10, 10, false);
    mycardPower.size(140, 40);
    lv_obj_move_foreground(mycardPower.obj);
    lv_obj_move_background(blackOverlay);
    lv_timer_create([](lv_timer_t *timer)
                    {
        if (animation_opa < 0)
            animation_opa = 0;
        if (animation_opa > 255)
            animation_opa = 255;
        lv_obj_set_style_opa(blackOverlay, animation_opa, 0);
        lv_label_set_text_fmt(lbl_power_off_prompt, "长按 %d 秒后关机", ((255 - animation_opa) >> 6) + 1);
        if (poweroff_started)
        {
            if (animation_opa == 255)
            {
                lv_label_set_text(lbl_power_off_prompt, "正在关机");
                lv_obj_fade_out(mycardPower.obj, 500, 0);
                doPoweroff();
                lv_timer_del(timer);
                return;
            }
            else
            {
                animation_opa += 2;
            }
        }
        else
        {
            if (animation_opa == 0)
            {
                animation_opa = 0;
                lv_obj_del(blackOverlay);
                blackOverlay = NULL;
                mycardPower.size(0, 0);
                lv_timer_del(timer);
                return;
            }
            else
            {
                animation_opa -= 8;
            }
        } }, 25, NULL);
    UNLOCKLV();
}

void refresh_poweroff_key()
{
    if (HAL::key_pressed[0] == true)
    {
        poweroff_started = true;
        lv_ui_poweroff();
    }
    else
    {
        poweroff_started = false;
    }
}
