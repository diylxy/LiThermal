#include <my_main.h>
#define BATTERY_CARD_X 250
#define BATTERY_CARD_SHOW_Y -13
#define BATTERY_CARD_HIDE_Y -43
#define BATTERY_CARD_WIDTH 56
#define BATTERY_CARD_WIDTH_CHARGING (56 + 12)
#define BATTERY_CARD_HEIGHT 33
extern "C" const lv_img_dsc_t bolt;

static MyCard card_Battery;
static lv_obj_t *img_bolt = NULL;

static bool expanded = false;
static void battery_card_construct(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_opa(parent, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_t *lbl_battery = lv_label_create(parent);
    lv_obj_set_align(lbl_battery, LV_ALIGN_TOP_LEFT);
    lv_obj_set_x(lbl_battery, -7);
    lv_label_set_text(lbl_battery, "0.00V");
    img_bolt = lv_img_create(parent);
    lv_img_set_src(img_bolt, &bolt);
    lv_obj_set_pos(img_bolt, 36, 2);
    lv_obj_set_style_opa(img_bolt, 0, 0);
}

static void battery_card_create()
{
    if (card_Battery.obj == NULL || lv_obj_is_valid(card_Battery.obj) == false)
    {
        card_Battery.create(lv_layer_sys(), BATTERY_CARD_X, BATTERY_CARD_HIDE_Y, BATTERY_CARD_WIDTH, BATTERY_CARD_HEIGHT, LV_ALIGN_TOP_LEFT);
        card_Battery.show(CARD_ANIM_NONE);
        battery_card_construct(card_Battery.obj);
    }
}

void battery_card_check()
{
    static int cnt = 0;
    static bool last_charging = false;
    if (current_mode == MODE_MAINMENU)
    {
        if (expanded == false)
        {
            expanded = true;
            LOCKLV();
            battery_card_create();
            card_Battery.move(BATTERY_CARD_X, BATTERY_CARD_SHOW_Y);
            UNLOCKLV();
            cnt = 20;
        }
        ++cnt;
        if (cnt >= 20)
        {
            int16_t voltage = PowerManager_getBatteryVoltage();
            if (voltage > 0)
            {
                LOCKLV();
                lv_label_set_text_fmt(lv_obj_get_child(card_Battery.obj, 0), "%d.%02dV", voltage / 1000, voltage % 1000 / 10);
                UNLOCKLV();
            }
            bool charging = PowerManager_isCharging();
            if (charging != last_charging)
            {
                last_charging = charging;
                if (charging)
                {
                    LOCKLV();
                    card_Battery.size(BATTERY_CARD_WIDTH_CHARGING, BATTERY_CARD_HEIGHT);
                    lv_obj_fade_in(img_bolt, 500, 0);
                    UNLOCKLV();
                }
                else
                {
                    LOCKLV();
                    card_Battery.size(BATTERY_CARD_WIDTH, BATTERY_CARD_HEIGHT);
                    lv_obj_fade_out(img_bolt, 300, 0);
                    UNLOCKLV();
                }
            }
            cnt = 0;
        }
    }
    else
    {
        if (expanded)
        {
            expanded = false;
            LOCKLV();
            card_Battery.move(BATTERY_CARD_X, BATTERY_CARD_HIDE_Y);
            UNLOCKLV();
        }
    }
}