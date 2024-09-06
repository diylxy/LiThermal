#pragma once
#include <lvgl/lvgl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.141592654
#endif // !M_PI
#define MY_POP_FALL_ANIM_DEFAULT_TIME 550
#define MY_MOVE_ANIM_DEFAULT_TIME 700
#define MY_SIZE_ANIM_DEFAULT_TIME 700
#define MY_FLY_ANIM_DEFAULT_HEIGHT 125
#define MY_FLY_UP_ANIM_DEFAULT_TIME 700
#define MY_FLY_DOWN_ANIM_DEFAULT_TIME 500
// 使用my_anim_pop_up函数添加的对象，禁止修改其userdata
void lv_my_anim_pop_up(lv_obj_t* obj, uint16_t time = MY_POP_FALL_ANIM_DEFAULT_TIME, uint16_t delay = 0);
// 使用lv_my_anim_fall_down函数添加的对象，禁止修改其userdata
void lv_my_anim_fall_down(lv_obj_t *obj, uint16_t time = MY_POP_FALL_ANIM_DEFAULT_TIME, uint16_t delay = 0, bool del = true);
void lv_anim_move(lv_obj_t* obj, lv_coord_t x, lv_coord_t y, uint16_t time = MY_MOVE_ANIM_DEFAULT_TIME, uint16_t delay = 0);
void lv_anim_size(lv_obj_t* obj, lv_coord_t w, lv_coord_t h, uint16_t time = MY_SIZE_ANIM_DEFAULT_TIME, uint16_t delay = 0);
void lv_anim_fly_up(lv_obj_t* obj, lv_coord_t h = MY_FLY_ANIM_DEFAULT_HEIGHT, uint16_t time = MY_FLY_UP_ANIM_DEFAULT_TIME, uint16_t delay = 0);
void lv_anim_fall_fly(lv_obj_t* obj, lv_coord_t h = MY_FLY_ANIM_DEFAULT_HEIGHT, uint16_t time = MY_FLY_DOWN_ANIM_DEFAULT_TIME, uint16_t delay = 0);

#define CARD_ANIM_NONE 0
#define CARD_ANIM_POP_UP 1
#define CARD_ANIM_FLY_UP 2
#define CARD_ANIM_FALL_DOWN 3
#define CARD_ANIM_FLY_DOWN 4
class MyCard
{
public:
    lv_obj_t *obj = NULL;
    MyCard(lv_obj_t *parent, lv_coord_t x = 0, lv_coord_t y = 0, lv_coord_t w = 0, lv_coord_t h = 0, lv_align_t align = LV_ALIGN_TOP_LEFT)
    {
        if (parent != NULL)
        {
            create(parent, x, y, w, h, align);
        }
    }
    MyCard(){}
    void create(lv_obj_t* parent, lv_coord_t x = 0, lv_coord_t y = 0, lv_coord_t w = 0, lv_coord_t h = 0, lv_align_t align = LV_ALIGN_TOP_LEFT)
    {
        if (obj != NULL)
            return;
        obj = lv_obj_create(parent);
        lv_obj_set_size(obj, w, h);
        lv_obj_align(obj, align, x, y);
        lv_obj_set_style_opa(obj, 0, 0);
    }
    void show(int anim_type = CARD_ANIM_POP_UP)
    {
        if (obj == NULL)
            return;
        if (anim_type == 0)
            lv_obj_set_style_opa(obj, LV_OPA_COVER, 0);
        else if(anim_type == 1)
            lv_my_anim_pop_up(obj);
        else if(anim_type == 2)
            lv_anim_fly_up(obj);
    }
    void hide(int anim_type = CARD_ANIM_FALL_DOWN)
    {
        if (obj == NULL)
            return;
        if (anim_type == CARD_ANIM_FALL_DOWN)
            lv_my_anim_fall_down(obj, MY_POP_FALL_ANIM_DEFAULT_TIME, 0, false);
        else if (anim_type == CARD_ANIM_FLY_DOWN)
            lv_anim_fall_fly(obj);
        else
            lv_obj_set_style_opa(obj, 0, 0);
    }
    void size(lv_coord_t w, lv_coord_t h, bool animated = true)
    {
        if (obj == NULL)
            return;
        if (animated)
            lv_anim_size(obj, w, h);
        else
            lv_obj_set_size(obj, w, h);
    }
    void move(lv_coord_t x, lv_coord_t y, bool animated = true)
    {
        if (obj == NULL)
            return;
        if(animated)
            lv_anim_move(obj, x, y);
        else
            lv_obj_set_pos(obj, x, y);
    }
    void del(int anim_type = CARD_ANIM_FALL_DOWN)
    {
        if (obj == NULL)
            return;
        if (lv_obj_is_valid(obj))
        {
            if (anim_type == CARD_ANIM_FALL_DOWN)
                lv_my_anim_fall_down(obj);
            else
                lv_obj_del(obj);
        }
        obj = NULL;
    }
};

