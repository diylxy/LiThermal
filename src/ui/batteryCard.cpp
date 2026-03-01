#include <my_main.h>
#define BATTERY_CARD_X 250
#define BATTERY_CARD_SHOW_Y -13
#define BATTERY_CARD_HIDE_Y -43
#define BATTERY_CARD_WIDTH 56
#define BATTERY_CARD_WIDTH_CHARGING (56 + 24)
#define BATTERY_CARD_HEIGHT 33
#define BATTERY_CARD_LABEL_X -7
#define BATTERY_CARD_CHILD_Y_PAD (-BATTERY_CARD_SHOW_Y)
#define BATTERY_CARD_LABEL_Y_UPSTREAM (BATTERY_CARD_CHILD_Y_PAD + 0)
#define BATTERY_CARD_BOLT_Y_UPSTREAM (BATTERY_CARD_CHILD_Y_PAD + 2)
extern "C" const lv_img_dsc_t bolt;

static MyCard card_Battery;
static lv_obj_t *img_bolt = NULL;

static int battery_card_calc_width(bool show_pct, bool charging)
{
    if (charging)
        return BATTERY_CARD_WIDTH_CHARGING;
    (void)show_pct;
    return BATTERY_CARD_WIDTH;
}

static int battery_card_calc_x(int w)
{
    return BATTERY_CARD_X + BATTERY_CARD_WIDTH - w;
}


static bool expanded = false;
static void battery_card_construct(lv_obj_t *parent)
{
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(parent, LV_OBJ_FLAG_OVERFLOW_VISIBLE);
    lv_obj_set_style_bg_opa(parent, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(parent, 0, 0);
    lv_obj_set_style_pad_all(parent, 0, 0);

    lv_obj_t *lbl_battery = lv_label_create(parent);
    lv_obj_set_align(lbl_battery, LV_ALIGN_TOP_LEFT);
    lv_obj_set_pos(lbl_battery, BATTERY_CARD_LABEL_X, BATTERY_CARD_LABEL_Y_UPSTREAM);
    lv_obj_set_style_text_align(lbl_battery, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(lbl_battery, LV_LABEL_LONG_CLIP);
    lv_label_set_text(lbl_battery, "0.00V");

    img_bolt = lv_img_create(parent);
    lv_img_set_src(img_bolt, &bolt);
    const int w = (int)lv_obj_get_width(parent);
    const int bolt_w = (int)bolt.header.w;
    const int bolt_x = w - bolt_w - 8;
    const int bolt_y = BATTERY_CARD_BOLT_Y_UPSTREAM;
    lv_obj_set_pos(img_bolt, bolt_x, bolt_y);
    lv_obj_set_style_opa(img_bolt, 0, 0);
}

static void battery_card_create()
{
    if (card_Battery.obj == NULL || lv_obj_is_valid(card_Battery.obj) == false)
    {
        const bool charging = PowerManager_isCharging();
        const bool show_pct = globalSettings.showBatteryPercent;
        const int w = battery_card_calc_width(show_pct, charging);

        card_Battery.create(lv_layer_sys(), battery_card_calc_x(w), BATTERY_CARD_HIDE_Y, w, BATTERY_CARD_HEIGHT, LV_ALIGN_TOP_LEFT);
        card_Battery.show(CARD_ANIM_NONE);
        battery_card_construct(card_Battery.obj);

        if (charging)
            lv_obj_set_style_opa(img_bolt, LV_OPA_COVER, 0);
    }
}

static int clamp_int(int v, int lo, int hi)
{
    if (v < lo)
        return lo;
    if (v > hi)
        return hi;
    return v;
}

static int battery_mv_to_percent(int mv)
{
    struct Pt
    {
        int mv;
        int pct;
    };

    static const Pt pts[] = {
        {4200, 100},
        {4100, 90},
        {4000, 80},
        {3900, 70},
        {3800, 60},
        {3700, 50},
        {3600, 30},
        {3500, 15},
        {3400, 5},
        {3300, 0},
    };

    mv = clamp_int(mv, pts[sizeof(pts) / sizeof(pts[0]) - 1].mv, pts[0].mv);

    for (size_t i = 0; i + 1 < sizeof(pts) / sizeof(pts[0]); i++)
    {
        const int hi_mv = pts[i].mv;
        const int lo_mv = pts[i + 1].mv;
        if (mv <= hi_mv && mv >= lo_mv)
        {
            const int hi_pct = pts[i].pct;
            const int lo_pct = pts[i + 1].pct;
            const int span_mv = hi_mv - lo_mv;
            if (span_mv <= 0)
                return clamp_int(lo_pct, 0, 100);
            const int num = (mv - lo_mv) * (hi_pct - lo_pct);
            const int pct = lo_pct + (num / span_mv);
            return clamp_int(pct, 0, 100);
        }
    }

    return 0;
}

void battery_card_check()
{
    static int cnt = 0;
    static bool last_charging = false;
    static bool last_show_pct = false; 
    static int filtered_mv = 0;
    static int disp_pct = -1;

    if (current_mode == MODE_MAINMENU)
    {
        if (expanded == false)
        {
            expanded = true;
            LOCKLV();
            battery_card_create();
            card_Battery.move(battery_card_calc_x(battery_card_calc_width(globalSettings.showBatteryPercent, PowerManager_isCharging())), BATTERY_CARD_SHOW_Y);
            UNLOCKLV();
            cnt = 20;
        }
        ++cnt;
        if (cnt >= 20)
        {
            const bool charging = PowerManager_isCharging();
            const bool show_pct_setting = globalSettings.showBatteryPercent;
            const bool show_pct = (show_pct_setting && !charging);
            const bool show_charging_text = (show_pct_setting && charging);
            if (show_pct != last_show_pct)
            {
                last_show_pct = show_pct;
                const int w = battery_card_calc_width(show_pct, charging);
                LOCKLV();
                card_Battery.size(w, BATTERY_CARD_HEIGHT);
                card_Battery.move(battery_card_calc_x(w), BATTERY_CARD_SHOW_Y, false);
                UNLOCKLV();
            }

            int16_t voltage = PowerManager_getBatteryVoltage();
            if (voltage > 0)
            {
                if (filtered_mv <= 0)
                    filtered_mv = voltage;
                else
                    filtered_mv = (filtered_mv * 8 + voltage * 2) / 10;

                int pct_raw = battery_mv_to_percent(filtered_mv);
                if (disp_pct < 0)
                {
                    disp_pct = pct_raw;
                }
                else
                {
                    if (pct_raw >= disp_pct + 2)
                        disp_pct += 1;
                    else if (pct_raw <= disp_pct - 2)
                        disp_pct -= 1;
                }

                disp_pct = clamp_int(disp_pct, 0, 100);

                LOCKLV();
                lv_obj_t *lbl = lv_obj_get_child(card_Battery.obj, 0);

                const int w = battery_card_calc_width(show_pct, charging);
                const int bolt_w = (int)bolt.header.w;
                const int bolt_x = w - bolt_w - 8;
                const int bolt_y = BATTERY_CARD_BOLT_Y_UPSTREAM;
                lv_obj_set_pos(img_bolt, bolt_x, bolt_y);
                lv_obj_set_align(lbl, LV_ALIGN_TOP_LEFT);
                lv_obj_set_x(lbl, BATTERY_CARD_LABEL_X);
                lv_obj_set_y(lbl, BATTERY_CARD_LABEL_Y_UPSTREAM);
                lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);

                if (show_charging_text)
                {
                    lv_obj_set_style_text_font(lbl, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);
                    lv_obj_set_width(lbl, bolt_x - BATTERY_CARD_LABEL_X);
                    lv_label_set_text(lbl, "充电中");
                }
                else if (show_pct)
                {
                    lv_obj_set_style_text_font(lbl, LV_FONT_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    const int label_w_full = w - 2 * BATTERY_CARD_LABEL_X;
                    lv_obj_set_width(lbl, label_w_full);
                    lv_label_set_text_fmt(lbl, "%d%%", disp_pct);
                }
                else
                {
                    lv_obj_set_style_text_font(lbl, LV_FONT_DEFAULT, LV_PART_MAIN | LV_STATE_DEFAULT);
                    if (charging)
                    {
                        lv_obj_set_width(lbl, bolt_x - BATTERY_CARD_LABEL_X);
                    }
                    else
                    {
                        const int label_w_full = w - 2 * BATTERY_CARD_LABEL_X;
                        lv_obj_set_width(lbl, label_w_full);
                    }
                    lv_label_set_text_fmt(lbl, "%d.%02dV", voltage / 1000, (voltage % 1000) / 10);
                }
                UNLOCKLV();
            }

            if (charging != last_charging)
            {
                last_charging = charging;
                last_show_pct = show_pct;

                const int w = battery_card_calc_width(show_pct, charging);
                LOCKLV();
                card_Battery.size(w, BATTERY_CARD_HEIGHT);
                card_Battery.move(battery_card_calc_x(w), BATTERY_CARD_SHOW_Y, false);
                if (charging)
                    lv_obj_fade_in(img_bolt, 500, 0);
                else
                    lv_obj_fade_out(img_bolt, 300, 0);
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
            const bool charging = PowerManager_isCharging();
            const bool show_pct = (globalSettings.showBatteryPercent && !charging);
            const int w = battery_card_calc_width(show_pct, charging);
            card_Battery.move(battery_card_calc_x(w), BATTERY_CARD_HIDE_Y);
            UNLOCKLV();
        }
    }
}