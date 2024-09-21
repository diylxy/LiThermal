#include "my_main.h"
static MyCard mycardMenuBasic;

static void card_settings_disp(lv_obj_t *ui_Panel1)
{
    /*
    lv_obj_set_width(ui_Panel1, 300);
    lv_obj_set_height(ui_Panel1, 200);
    lv_obj_set_x(ui_Panel1, 10);
    lv_obj_set_y(ui_Panel1, 48);
*/
    lv_obj_clear_flag(ui_Panel1, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_text_font(ui_Panel1, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *ui_Checkbox2 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox2, "最大值");
    lv_obj_set_width(ui_Checkbox2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Checkbox2, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Checkbox2, -95);
    lv_obj_set_y(ui_Checkbox2, 0);
    lv_obj_set_align(ui_Checkbox2, LV_ALIGN_TOP_MID);

    lv_obj_t *ui_Checkbox5 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox5, "最小值");
    lv_obj_set_width(ui_Checkbox5, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Checkbox5, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Checkbox5, LV_ALIGN_TOP_MID);

    lv_obj_t *ui_Checkbox3 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox3, "中心值");
    lv_obj_set_width(ui_Checkbox3, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Checkbox3, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Checkbox3, 95);
    lv_obj_set_y(ui_Checkbox3, 0);
    lv_obj_set_align(ui_Checkbox3, LV_ALIGN_TOP_MID);

    // 图表控制
    lv_obj_t *ui_Checkbox4 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox4, "显示图表");
    lv_obj_set_width(ui_Checkbox4, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Checkbox4, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Checkbox4, -87);
    lv_obj_set_y(ui_Checkbox4, 38);
    lv_obj_set_align(ui_Checkbox4, LV_ALIGN_TOP_MID);

    lv_obj_t *ui_Checkbox1 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox1, "大尺寸");
    lv_obj_set_width(ui_Checkbox1, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Checkbox1, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Checkbox1, 7);
    lv_obj_set_y(ui_Checkbox1, 38);
    lv_obj_set_align(ui_Checkbox1, LV_ALIGN_TOP_MID);

    lv_obj_t *ui_Roller1 = lv_roller_create(ui_Panel1);
    lv_roller_set_options(ui_Roller1, "40 ms\n100 ms\n500 ms\n1 s\n2 s\n5 s\n10 s", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_Roller1, 85);
    lv_obj_set_height(ui_Roller1, 75);
    lv_obj_set_x(ui_Roller1, -95);
    lv_obj_set_y(ui_Roller1, 95);
    lv_obj_set_align(ui_Roller1, LV_ALIGN_TOP_MID);

    lv_obj_t *ui_Roller2 = lv_roller_create(ui_Panel1);
    lv_roller_set_options(ui_Roller2, "左上\n右上\n右下\n左下", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_Roller2, 85);
    lv_obj_set_height(ui_Roller2, 75);
    lv_obj_set_x(ui_Roller2, 0);
    lv_obj_set_y(ui_Roller2, 95);
    lv_obj_set_align(ui_Roller2, LV_ALIGN_TOP_MID);

    lv_obj_t *ui_Roller3 = lv_roller_create(ui_Panel1);
    lv_roller_set_options(ui_Roller3,
                          "白热\n黑热\n融合1\n融合2\n彩虹\n铁红1\n铁红2\n深褐色\n色彩1\n色彩2\n冰火\n雨\n红热\n绿热\n深蓝",
                          LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_Roller3, 85);
    lv_obj_set_height(ui_Roller3, 132);
    lv_obj_set_x(ui_Roller3, 95);
    lv_obj_set_y(ui_Roller3, 38);
    lv_obj_set_align(ui_Roller3, LV_ALIGN_TOP_MID);

    lv_obj_t *ui_Label1 = lv_label_create(ui_Panel1);
    lv_obj_set_x(ui_Label1, -95);
    lv_obj_set_y(ui_Label1, 72);
    lv_obj_set_align(ui_Label1, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label1, "速率");

    lv_obj_t *ui_Label3 = lv_label_create(ui_Panel1);
    lv_obj_set_x(ui_Label3, 0);
    lv_obj_set_y(ui_Label3, 72);
    lv_obj_set_align(ui_Label3, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label3, "位置");
    lv_obj_set_style_bg_opa(ui_Panel1, 192, 0);
    lv_obj_set_style_border_width(ui_Panel1, 0, 0);
    // lv_obj_set_style_bg_opa(ui_Checkbox1, 192, LV_PART_INDICATOR);
    // lv_obj_set_style_bg_opa(ui_Checkbox2, 192, LV_PART_INDICATOR);
    // lv_obj_set_style_bg_opa(ui_Checkbox3, 192, LV_PART_INDICATOR);
    // lv_obj_set_style_bg_opa(ui_Checkbox4, 192, LV_PART_INDICATOR);
    // lv_obj_set_style_bg_opa(ui_Checkbox5, 192, LV_PART_INDICATOR);
    // lv_obj_set_style_bg_opa(ui_Roller1, 192, 0);
    // lv_obj_set_style_bg_opa(ui_Roller2, 192, 0);
    // lv_obj_set_style_bg_opa(ui_Roller3, 192, 0);

    // 设置回调
    lv_obj_add_event_cb(ui_Checkbox1, [](lv_event_t *e)
                        { globalSettings.graphSize = lv_obj_has_state(e->target, LV_STATE_CHECKED); widget_graph_updateSettings(); }, LV_EVENT_CLICKED, NULL); // 大尺寸
    lv_obj_add_event_cb(ui_Checkbox2, [](lv_event_t *e)
                        { globalSettings.enableMaxValueDisplay = lv_obj_has_state(e->target, LV_STATE_CHECKED); ui_crosshairs_updateVisibility(); }, LV_EVENT_CLICKED, NULL); // 最大值
    lv_obj_add_event_cb(ui_Checkbox3, [](lv_event_t *e)
                        { globalSettings.enableCenterValueDisplay = lv_obj_has_state(e->target, LV_STATE_CHECKED); }, LV_EVENT_CLICKED, NULL); // 中心值
    lv_obj_add_event_cb(ui_Checkbox4, [](lv_event_t *e)
                        { globalSettings.enableGraph = lv_obj_has_state(e->target, LV_STATE_CHECKED); widget_graph_updateSettings(); }, LV_EVENT_CLICKED, NULL); // 显示图表
    lv_obj_add_event_cb(ui_Checkbox5, [](lv_event_t *e)
                        { globalSettings.enableMinValueDisplay = lv_obj_has_state(e->target, LV_STATE_CHECKED); ui_crosshairs_updateVisibility(); }, LV_EVENT_CLICKED, NULL); // 最小值
    lv_obj_add_event_cb(ui_Roller1, [](lv_event_t *e)
                        { globalSettings.graphRefreshInterval = lv_roller_get_selected(e->target); widget_graph_updateSettings(); }, LV_EVENT_KEY, NULL); // 数据源
    lv_obj_add_event_cb(ui_Roller2, [](lv_event_t *e)
                        { globalSettings.graphPos = lv_roller_get_selected(e->target); widget_graph_updateSettings(); }, LV_EVENT_KEY, NULL); // 位置
    lv_obj_add_event_cb(ui_Roller3, [](lv_event_t *e)
                        { globalSettings.colorPalette = lv_roller_get_selected(e->target); }, LV_EVENT_KEY, NULL); // 颜色

    // 当前设置
    if (globalSettings.graphSize)
        lv_obj_add_state(ui_Checkbox1, LV_STATE_CHECKED);
    if (globalSettings.enableMaxValueDisplay)
        lv_obj_add_state(ui_Checkbox2, LV_STATE_CHECKED);
    if (globalSettings.enableCenterValueDisplay)
        lv_obj_add_state(ui_Checkbox3, LV_STATE_CHECKED);
    if (globalSettings.enableGraph)
        lv_obj_add_state(ui_Checkbox4, LV_STATE_CHECKED);
    if (globalSettings.enableMinValueDisplay)
        lv_obj_add_state(ui_Checkbox5, LV_STATE_CHECKED);
    lv_roller_set_selected(ui_Roller1, globalSettings.graphRefreshInterval, LV_ANIM_OFF);
    lv_roller_set_selected(ui_Roller2, globalSettings.graphPos, LV_ANIM_OFF);
    lv_roller_set_selected(ui_Roller3, globalSettings.colorPalette, LV_ANIM_OFF);
}

#define MENU_BASIC_CARD_WIDTH 300
#define MENU_BASIC_CARD_HEIGHT 200
#define MENU_BASIC_CARD_POS_Y 30

void menu_basic_show()
{
    mycardMenuBasic.create(lv_layer_top(), 0, MENU_BASIC_CARD_POS_Y, MENU_BASIC_CARD_WIDTH, MENU_BASIC_CARD_HEIGHT, LV_ALIGN_TOP_MID);
    card_settings_disp(mycardMenuBasic.obj);
    lv_group_focus_obj(lv_obj_get_child(mycardMenuBasic.obj, 0));
    mycardMenuBasic.show();
}

void menu_basic_hide()
{
    mycardMenuBasic.del();
    settings_save();
}
