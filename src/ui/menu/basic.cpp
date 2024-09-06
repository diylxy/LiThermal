#include "my_main.h"
static MyCard mycardMenuBasic;

static void card_settings_disp(lv_obj_t* ui_Panel1) {
    /*
    lv_obj_set_width(ui_Panel1, 300);
    lv_obj_set_height(ui_Panel1, 200);
    lv_obj_set_x(ui_Panel1, 10);
    lv_obj_set_y(ui_Panel1, 48);
*/
    lv_obj_clear_flag(ui_Panel1, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
    lv_obj_set_style_text_font(ui_Panel1, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t *ui_Checkbox2 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox2, "最大值");
    lv_obj_set_width(ui_Checkbox2, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Checkbox2, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Checkbox2, -95);
    lv_obj_set_y(ui_Checkbox2, 0);
    lv_obj_set_align(ui_Checkbox2, LV_ALIGN_TOP_MID);

    lv_obj_t* ui_Checkbox5 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox5, "最小值");
    lv_obj_set_width(ui_Checkbox5, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Checkbox5, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_align(ui_Checkbox5, LV_ALIGN_TOP_MID);

    lv_obj_t* ui_Checkbox3 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox3, "中心值");
    lv_obj_set_width(ui_Checkbox3, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Checkbox3, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Checkbox3, 95);
    lv_obj_set_y(ui_Checkbox3, 0);
    lv_obj_set_align(ui_Checkbox3, LV_ALIGN_TOP_MID);

    // 图表控制
    lv_obj_t* ui_Checkbox4 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox4, "显示图表");
    lv_obj_set_width(ui_Checkbox4, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Checkbox4, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Checkbox4, -87);
    lv_obj_set_y(ui_Checkbox4, 38);
    lv_obj_set_align(ui_Checkbox4, LV_ALIGN_TOP_MID);

    lv_obj_t* ui_Checkbox1 = lv_checkbox_create(ui_Panel1);
    lv_checkbox_set_text(ui_Checkbox1, "大尺寸");
    lv_obj_set_width(ui_Checkbox1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Checkbox1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Checkbox1, 7);
    lv_obj_set_y(ui_Checkbox1, 38);
    lv_obj_set_align(ui_Checkbox1, LV_ALIGN_TOP_MID);

    lv_obj_t* ui_Roller1 = lv_roller_create(ui_Panel1);
    lv_roller_set_options(ui_Roller1, "最大值\n最小值\n中心值", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_Roller1, 85);
    lv_obj_set_height(ui_Roller1, 75);
    lv_obj_set_x(ui_Roller1, -95);
    lv_obj_set_y(ui_Roller1, 95);
    lv_obj_set_align(ui_Roller1, LV_ALIGN_TOP_MID);

    lv_obj_t* ui_Roller2 = lv_roller_create(ui_Panel1);
    lv_roller_set_options(ui_Roller2, "左上\n右上\n右下\n左下", LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_Roller2, 85);
    lv_obj_set_height(ui_Roller2, 75);
    lv_obj_set_x(ui_Roller2, 0);
    lv_obj_set_y(ui_Roller2, 95);
    lv_obj_set_align(ui_Roller2, LV_ALIGN_TOP_MID);

    lv_obj_t* ui_Roller3 = lv_roller_create(ui_Panel1);
    lv_roller_set_options(ui_Roller3,
        "白热\n黑热\n融合1\n彩虹\n融合2\n铁红1\n铁红2\n深褐色\n色彩1\n色彩2\n冰火\n雨\n红热\n绿热\n深蓝",
        LV_ROLLER_MODE_INFINITE);
    lv_obj_set_width(ui_Roller3, 85);
    lv_obj_set_height(ui_Roller3, 132);
    lv_obj_set_x(ui_Roller3, 95);
    lv_obj_set_y(ui_Roller3, 38);
    lv_obj_set_align(ui_Roller3, LV_ALIGN_TOP_MID);

    lv_obj_t* ui_Label1 = lv_label_create(ui_Panel1);
    lv_obj_set_x(ui_Label1, -95);
    lv_obj_set_y(ui_Label1, 72);
    lv_obj_set_align(ui_Label1, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label1, "数据源");

    lv_obj_t* ui_Label3 = lv_label_create(ui_Panel1);
    lv_obj_set_x(ui_Label3, 0);
    lv_obj_set_y(ui_Label3, 72);
    lv_obj_set_align(ui_Label3, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label3, "位置");

}

#define MENU_BASIC_CARD_WIDTH 300
#define MENU_BASIC_CARD_HEIGHT 200
#define MENU_BASIC_CARD_POS_Y 30

void menu_basic_show()
{
    mycardMenuBasic.create(lv_layer_top(), 0, 0, 0, 0, LV_ALIGN_TOP_MID);
    card_settings_disp(mycardMenuBasic.obj);
    mycardMenuBasic.move(0, MENU_BASIC_CARD_POS_Y);
    mycardMenuBasic.size(MENU_BASIC_CARD_WIDTH, MENU_BASIC_CARD_HEIGHT);
}

void menu_basic_hide()
{
    mycardMenuBasic.move(0, 0);
    mycardMenuBasic.size(0, 0);
    lv_obj_del_delayed(mycardMenuBasic.obj, 800);
    mycardMenuBasic.obj = NULL;
}
