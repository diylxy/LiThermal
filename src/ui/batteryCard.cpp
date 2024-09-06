#include <my_main.h>
#define BATTERY_CARD_X 250
#define BATTERY_CARD_SHOW_Y -13
#define BATTERY_CARD_HIDE_Y -43

static MyCard card_Battery;
static bool expanded = false;
static void battery_card_construct(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_opa(parent, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_t *lbl_battery = lv_label_create(parent);
    lv_obj_set_align(lbl_battery, LV_ALIGN_TOP_MID);
    lv_label_set_text(lbl_battery, "NA");
}

static void battery_card_create()
{
    if (card_Battery.obj == NULL || lv_obj_is_valid(card_Battery.obj) == false)
    {
        card_Battery.create(lv_layer_sys(), BATTERY_CARD_X, BATTERY_CARD_HIDE_Y, 56, 33, LV_ALIGN_TOP_LEFT);
        card_Battery.show(CARD_ANIM_NONE);
        battery_card_construct(card_Battery.obj);
    }
}

void battery_card_check()
{
    static int cnt = 0;
    if (current_mode == MODE_MAINMENU)
    {
        if (expanded == false)
        {
            expanded = true;
            LOCKLV();
            battery_card_create();
            card_Battery.move(BATTERY_CARD_X, BATTERY_CARD_SHOW_Y);
            UNLOCKLV();
            cnt = 10;
        }
        ++cnt;
        if (cnt >= 10)
        {
            int16_t voltage = PowerManager_getBatteryVoltage();
            if (voltage > 0)
            {
                LOCKLV();
                lv_label_set_text_fmt(lv_obj_get_child(card_Battery.obj, 0), "%d.%02dV", voltage / 1000, voltage % 1000 / 10);
                UNLOCKLV();
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