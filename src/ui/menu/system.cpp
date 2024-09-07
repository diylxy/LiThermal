#include "my_main.h"
static MyCard mycardMenuSystem;

#define MENU_SYSTEM_CARD_WIDTH 300
#define MENU_SYSTEM_CARD_HEIGHT 220
#define MENU_SYSTEM_CARD_POS_Y 30
static void card_menu_system_construct(lv_obj_t *parent)
{

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
}
