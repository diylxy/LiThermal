#include "my_main.h"
static MyCard mycardMenuSystem;

#define MENU_SYSTEM_CARD_WIDTH 300
#define MENU_SYSTEM_CARD_HEIGHT 220
#define MENU_SYSTEM_CARD_POS_Y 30
static int factory_reset_cnt;
static lv_obj_t *ui_Switch2;
static lv_obj_t *ui_Switch1;
static void card_menu_system_construct(lv_obj_t *parent)
{
    factory_reset_cnt = 3;
    // ui_Panel2 = lv_obj_create(ui_Screen4);
    // lv_obj_set_width(ui_Panel2, 300);
    // lv_obj_set_height(ui_Panel2, 220);
    // lv_obj_set_x(ui_Panel2, 0);
    // lv_obj_set_y(ui_Panel2, 30);
    // lv_obj_set_align(ui_Panel2, LV_ALIGN_TOP_MID);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_text_font(parent, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Panel5 = lv_obj_create(parent);
    lv_obj_set_width(ui_Panel5, 270);
    lv_obj_set_height(ui_Panel5, 42);
    lv_obj_clear_flag(ui_Panel5, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_Switch1 = lv_switch_create(ui_Panel5);
    lv_obj_set_width(ui_Switch1, 50);
    lv_obj_set_height(ui_Switch1, 25);
    lv_obj_set_align(ui_Switch1, LV_ALIGN_RIGHT_MID);

    lv_obj_t *ui_Label11 = lv_label_create(ui_Panel5);
    lv_obj_set_width(ui_Label11, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label11, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label11, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_Label11, "照片保留UI");
    lv_obj_set_style_text_font(ui_Label11, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Panel10 = lv_obj_create(parent);
    lv_obj_set_width(ui_Panel10, 270);
    lv_obj_set_height(ui_Panel10, 42);
    lv_obj_set_x(ui_Panel10, 0);
    lv_obj_set_y(ui_Panel10, 50);
    lv_obj_clear_flag(ui_Panel10, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_Switch2 = lv_switch_create(ui_Panel10);
    lv_obj_set_width(ui_Switch2, 50);
    lv_obj_set_height(ui_Switch2, 25);
    lv_obj_set_align(ui_Switch2, LV_ALIGN_RIGHT_MID);

    lv_obj_t *ui_Label14 = lv_label_create(ui_Panel10);
    lv_obj_set_width(ui_Label14, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label14, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label14, LV_ALIGN_LEFT_MID);
    lv_label_set_text(ui_Label14, "拍照动画反色");
    lv_obj_set_style_text_font(ui_Label14, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Button1 = lv_btn_create(parent);
    lv_obj_set_width(ui_Button1, 140);
    lv_obj_set_height(ui_Button1, 40);
    lv_obj_set_x(ui_Button1, 0);
    lv_obj_set_y(ui_Button1, 100);
    lv_obj_add_flag(ui_Button1, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_Button1, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_Button1, lv_color_hex(0xE28D00), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Button1, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Label12 = lv_label_create(ui_Button1);
    lv_obj_set_width(ui_Label12, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label12, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label12, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label12, "手动清除噪声");
    lv_obj_set_style_text_font(ui_Label12, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Button6 = lv_btn_create(parent);
    lv_obj_set_width(ui_Button6, 120);
    lv_obj_set_height(ui_Button6, 40);
    lv_obj_set_x(ui_Button6, 150);
    lv_obj_set_y(ui_Button6, 100);
    lv_obj_add_flag(ui_Button6, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_Button6, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_Button6, lv_color_hex(0xF44336), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Button6, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Label13 = lv_label_create(ui_Button6);
    lv_obj_set_width(ui_Label13, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label13, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label13, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label13, "恢复默认设置");
    lv_obj_set_style_text_font(ui_Label13, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Label15 = lv_label_create(parent);
    lv_obj_set_width(ui_Label15, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label15, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label15, 0);
    lv_obj_set_y(ui_Label15, 150);
    lv_obj_set_align(ui_Label15, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label15, "作者：小李电子实验室\n开源协议：GPL 3.0");
    lv_obj_set_style_text_color(ui_Label15, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui_Label15, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui_Label15, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui_Label15, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_group_focus_obj(ui_Button1);

    lv_obj_add_event_cb(ui_Switch1, [](lv_event_t *e)
                        { globalSettings.preserveUI = lv_obj_has_state(e->target, LV_STATE_CHECKED); }, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_Switch2, [](lv_event_t *e)
                        { globalSettings.useBlackFlashBang = lv_obj_has_state(e->target, LV_STATE_CHECKED); }, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_Button1, [](lv_event_t *e)
                        { cameraUtils.calibrateManually(); }, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_Button6, [](lv_event_t *e)
                        {
        if (factory_reset_cnt == 0)
        {
            factory_reset_cnt = 4;
            lv_label_set_text(lv_obj_get_child(e->target, 0), "已恢复");
            settings_default();
            settings_save();
            lv_obj_clear_state(ui_Switch1, LV_STATE_CHECKED);
            lv_obj_clear_state(ui_Switch2, LV_STATE_CHECKED);
            widget_graph_updateSettings();
            ui_crosshairs_updateVisibility();
        }
        else
        {
            lv_label_set_text_fmt(lv_obj_get_child(e->target, 0), "再按%d次后恢复", factory_reset_cnt);
        }
        --factory_reset_cnt; }, LV_EVENT_CLICKED, NULL);

    if (globalSettings.preserveUI)
        lv_obj_add_state(ui_Switch1, LV_STATE_CHECKED);
    if (globalSettings.useBlackFlashBang)
        lv_obj_add_state(ui_Switch2, LV_STATE_CHECKED);
}

void menu_system_show()
{
    mycardMenuSystem.create(lv_layer_top(), 0, MENU_SYSTEM_CARD_POS_Y, MENU_SYSTEM_CARD_WIDTH, MENU_SYSTEM_CARD_HEIGHT, LV_ALIGN_TOP_MID);
    card_menu_system_construct(mycardMenuSystem.obj);
    lv_group_focus_obj(lv_obj_get_child(mycardMenuSystem.obj, 0));
    mycardMenuSystem.show(CARD_ANIM_FLY_UP);
}

void menu_system_hide()
{
    mycardMenuSystem.del(CARD_ANIM_FLY_DOWN);
    settings_save();
}
