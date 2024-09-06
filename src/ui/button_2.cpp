#include <my_main.h>

// 处理中键按下事件
#define MENU_BUTTON_2_BOX_WIDHT 230
#define MENU_BUTTON_2_BOX_HEIGHT 136
#define MENU_BUTTON_2_BOX_Y_SHOW 20
#define MENU_BUTTON_2_BUTTON_WIDTH_DEFAULT 60
#define MENU_BUTTON_2_BUTTON_HEIGHT_DEFAULT 100
#define MENU_BUTTON_2_BUTTON_WIDTH_FOCUSED 70
#define MENU_BUTTON_2_BUTTON_HEIGHT_FOCUSED 110
static MyCard card_menuPage;
static bool close_for_opening_submenu = false;
static int selected_menu_number = -1;
static bool expanded = false;

static void lv_anim_size_ease_out(lv_obj_t *obj, lv_coord_t w, lv_coord_t h, uint16_t time, uint16_t delay)
{
    lv_anim_t a;
    int16_t p;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_path_cb(&a, my_anim_path_ease_out);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    p = lv_obj_get_style_width(obj, 0);
    if (p != w)
    {
        lv_anim_set_values(&a, p, w);
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_width);
        lv_anim_start(&a);
    }
    p = lv_obj_get_style_height(obj, 0);
    if (p != h)
    {
        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_height);
        lv_anim_set_values(&a, p, h);
        lv_anim_start(&a);
    }
}

void event_button_clicked_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    int code = e->code;
    switch (code)
    {
    case LV_EVENT_CLICKED:
        selected_menu_number = reinterpret_cast<int>(e->user_data);
        close_for_opening_submenu = true;
        break;
    case LV_EVENT_FOCUSED:
        if (expanded)
            lv_anim_size_ease_out(target, MENU_BUTTON_2_BUTTON_WIDTH_FOCUSED, MENU_BUTTON_2_BUTTON_HEIGHT_FOCUSED, 500, 0);
        break;
    case LV_EVENT_DEFOCUSED:
        if (expanded)
            lv_anim_size_ease_out(target, MENU_BUTTON_2_BUTTON_WIDTH_DEFAULT, MENU_BUTTON_2_BUTTON_HEIGHT_DEFAULT, 500, 0);
        break;
    default:
        break;
    }
}

void menu_button_2_construct(lv_obj_t *parent)
{
    lv_obj_set_align(parent, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(parent, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
    lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_opa(parent, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(parent, 0, 0);

    lv_obj_t *ui_Button2 = lv_btn_create(parent);
    lv_obj_set_width(ui_Button2, MENU_BUTTON_2_BUTTON_WIDTH_DEFAULT);
    lv_obj_set_height(ui_Button2, MENU_BUTTON_2_BUTTON_HEIGHT_DEFAULT);
    lv_obj_set_x(ui_Button2, -69);
    lv_obj_set_y(ui_Button2, 2);
    lv_obj_set_align(ui_Button2, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_Button2, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_Button2, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_Button2, lv_color_hex(0x282B30), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Button2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Button2, lv_color_hex(0x0081B3), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(ui_Button2, 255, LV_PART_MAIN | LV_STATE_FOCUSED);

    lv_obj_t *ui_Label4 = lv_label_create(ui_Button2);
    lv_obj_set_width(ui_Label4, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label4, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label4, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label4, "");
    lv_obj_set_style_text_font(ui_Label4, &ui_font_fontawesome24, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Label5 = lv_label_create(ui_Button2);
    lv_obj_set_width(ui_Label5, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label5, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label5, 0);
    lv_obj_set_y(ui_Label5, 40);
    lv_obj_set_align(ui_Label5, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label5, "相册");
    lv_obj_set_style_text_font(ui_Label5, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Button3 = lv_btn_create(parent);
    lv_obj_set_width(ui_Button3, MENU_BUTTON_2_BUTTON_WIDTH_DEFAULT);
    lv_obj_set_height(ui_Button3, MENU_BUTTON_2_BUTTON_HEIGHT_DEFAULT);
    lv_obj_set_x(ui_Button3, -1);
    lv_obj_set_y(ui_Button3, 3);
    lv_obj_set_align(ui_Button3, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_Button3, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_Button3, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_Button3, lv_color_hex(0x282B30), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Button3, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Button3, lv_color_hex(0xBB9100), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(ui_Button3, 255, LV_PART_MAIN | LV_STATE_FOCUSED);

    lv_obj_t *ui_Label6 = lv_label_create(ui_Button3);
    lv_obj_set_width(ui_Label6, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label6, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label6, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label6, "");
    lv_obj_set_style_text_font(ui_Label6, &ui_font_fontawesome24, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Label7 = lv_label_create(ui_Button3);
    lv_obj_set_width(ui_Label7, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label7, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label7, 0);
    lv_obj_set_y(ui_Label7, 40);
    lv_obj_set_align(ui_Label7, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label7, "相机\n设置");
    lv_obj_set_style_text_font(ui_Label7, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Button4 = lv_btn_create(parent);
    lv_obj_set_width(ui_Button4, MENU_BUTTON_2_BUTTON_WIDTH_DEFAULT);
    lv_obj_set_height(ui_Button4, MENU_BUTTON_2_BUTTON_HEIGHT_DEFAULT);
    lv_obj_set_x(ui_Button4, 67);
    lv_obj_set_y(ui_Button4, 3);
    lv_obj_set_align(ui_Button4, LV_ALIGN_TOP_MID);
    lv_obj_add_flag(ui_Button4, LV_OBJ_FLAG_SCROLL_ON_FOCUS); /// Flags
    lv_obj_clear_flag(ui_Button4, LV_OBJ_FLAG_SCROLLABLE);    /// Flags
    lv_obj_set_style_bg_color(ui_Button4, lv_color_hex(0x282B30), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Button4, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_Button4, lv_color_hex(0x960000), LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_set_style_bg_opa(ui_Button4, 255, LV_PART_MAIN | LV_STATE_FOCUSED);

    lv_obj_t *ui_Label8 = lv_label_create(ui_Button4);
    lv_obj_set_width(ui_Label8, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label8, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Label8, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label8, "");
    lv_obj_set_style_text_font(ui_Label8, &ui_font_fontawesome24, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *ui_Label9 = lv_label_create(ui_Button4);
    lv_obj_set_width(ui_Label9, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Label9, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_Label9, 0);
    lv_obj_set_y(ui_Label9, 40);
    lv_obj_set_align(ui_Label9, LV_ALIGN_TOP_MID);
    lv_label_set_text(ui_Label9, "系统\n设置");
    lv_obj_set_style_text_font(ui_Label9, &ui_font_chinese16, LV_PART_MAIN | LV_STATE_DEFAULT);

    // events
    lv_obj_add_event_cb(ui_Button2, event_button_clicked_cb, LV_EVENT_ALL, reinterpret_cast<void *>(0));
    lv_obj_add_event_cb(ui_Button3, event_button_clicked_cb, LV_EVENT_ALL, reinterpret_cast<void *>(1));
    lv_obj_add_event_cb(ui_Button4, event_button_clicked_cb, LV_EVENT_ALL, reinterpret_cast<void *>(2));
}

static void card_menuPage_create()
{
    if (card_menuPage.obj == NULL || lv_obj_is_valid(card_menuPage.obj) == false)
    {
        card_menuPage.create(lv_layer_sys(), -50, 0, 0, 0, LV_ALIGN_TOP_MID);
        card_menuPage.show(CARD_ANIM_NONE);
        menu_button_2_construct(card_menuPage.obj);
        lv_group_focus_obj(lv_obj_get_child(card_menuPage.obj, 0));
    }
}

void refresh_menu_key()
{
    if (expanded == false)
    {
        if (HAL::key_press_event[1] == true && IN_MAINPAGE)
        {
            expanded = true;
            close_for_opening_submenu = false;
            HAL::key_press_event[1] = false;
            LOCKLV();
            card_menuPage_create();
            card_menuPage.move(0, MENU_BUTTON_2_BOX_Y_SHOW);
            card_menuPage.size(MENU_BUTTON_2_BOX_WIDHT, MENU_BUTTON_2_BOX_HEIGHT);
            EXIT_MAINPAGE();
            UNLOCKLV();
        }
        else
        {
            if (card_menuPage.obj != NULL && lv_obj_is_valid(card_menuPage.obj))
            {
                if (lv_obj_get_width(card_menuPage.obj) == 0)
                {
                    // 已收起，删除widget
                    LOCKLV();
                    lv_obj_del(card_menuPage.obj);
                    card_menuPage.obj = NULL;
                    UNLOCKLV();
                }
            }
        }
    }
    else
    {
        if (close_for_opening_submenu || (HAL::key_press_event[1] == true))
        {
            if (close_for_opening_submenu == false)
            {
                BACK_TO_MAINPAGE();
            }
            else
            {
                printf("Clicked %d\n", selected_menu_number);
                BACK_TO_MAINPAGE();
                // open menu selected_menu_number
            }
            selected_menu_number = -1;
            close_for_opening_submenu = false;
            HAL::key_press_event[1] = false;
            expanded = false;
            card_menuPage.size(0, 0);
            card_menuPage.move(-50, 0);
        }
    }
}