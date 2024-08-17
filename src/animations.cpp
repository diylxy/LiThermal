#include "animations.h"
float my_ease_bounce_back(float t)
{
    return 1 - (1 - t) * (1 - t) * cos(1.5 * M_PI * t);
}
float my_ease_out(float t)
{
    return 1 - (1 - t) * (1 - t) * (1 - t);
}
float my_ease_in(float t)
{
    t = t * t;
    return t * t;
}
float easeInOutQuart(float t) {
    if (t < 0.5) {
        t *= t;
        return 8 * t * t;
    }
    else {
        t = (--t) * t;
        return 1 - 8 * t * t;
    }
}
int32_t my_anim_path_ease_in_out(const lv_anim_t* a)
{
    /*Calculate the current step*/
    float t = (float)(a->act_time) / (float)(a->time);
    float new_value;
    new_value = easeInOutQuart(t) * (a->end_value - a->start_value) + a->start_value;
    return new_value;
}

int32_t my_anim_path_ease_out(const lv_anim_t* a)
{
    /*Calculate the current step*/
    float t = (float)(a->act_time) / (float)(a->time);
    float new_value;
    new_value = my_ease_out(t) * (a->end_value - a->start_value) + a->start_value;
    return new_value;
}

int32_t my_anim_path_ease_in(const lv_anim_t* a)
{
    /*Calculate the current step*/
    float t = (float)(a->act_time) / (float)(a->time);
    float new_value;
    new_value = my_ease_in(t) * (a->end_value - a->start_value) + a->start_value;
    return new_value;
}

void lv_my_anim_cb_pop_up(void* obj, int val)
{
    lv_obj_t* target_obj = (lv_obj_t*)obj;
    lv_coord_t y_start =  (lv_coord_t)((int)lv_obj_get_user_data(target_obj));
    int current_y = lv_obj_get_style_y(target_obj, 0);
    float ang = my_ease_out(val / 100.0) * (40) - 40;
    float y_pos = my_ease_bounce_back(val / 100.0) * (-40) + 40 + y_start;
    float opa = val / 100.0 * 200 + 55;
    lv_obj_set_y(target_obj, y_pos);
    lv_obj_set_style_transform_angle(target_obj, ang, 0);
    lv_obj_set_style_opa(target_obj, opa, 0);
}
void lv_my_anim_cb_fall_down(void* obj, int val)
{
    lv_obj_t* target_obj = (lv_obj_t*)obj;
    lv_coord_t y_start = (lv_coord_t)((int)lv_obj_get_user_data(target_obj));
    int current_y = lv_obj_get_style_y(target_obj, 0);
    float ang = my_ease_out(val / 100.0) * (40);
    float y_pos = my_ease_out(val / 100.0) * (40) + y_start;
    float opa = 255.0 - val * 255.0 / 40.0;
    if (opa < 0)
        opa = 0;
    lv_obj_set_y(target_obj, y_pos);
    lv_obj_set_style_transform_angle(target_obj, ang, 0);
    lv_obj_set_style_opa(target_obj, opa, 0);
}
// 使用my_anim_pop_up函数添加的对象，禁止修改其userdata
void lv_my_anim_pop_up(lv_obj_t* obj, uint16_t time, uint16_t delay)
{
    static lv_anim_t a;
    lv_coord_t y_start;
    lv_obj_set_style_transform_pivot_x(obj, lv_obj_get_width(obj), 0);
    lv_anim_init(&a);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_exec_cb(&a, lv_my_anim_cb_pop_up);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_ready_cb(&a, [](lv_anim_t* a)
        { lv_obj_set_style_transform_angle((lv_obj_t *)a->var, 0, 0); });
    lv_anim_set_var(&a, obj);
    y_start = lv_obj_get_style_y(obj, 0);
    lv_obj_set_user_data(obj, reinterpret_cast<void *>(y_start));
    if(lv_obj_get_style_opa(obj, 0) > 253)
        lv_obj_set_style_opa(obj, LV_OPA_TRANSP, 0);
    lv_anim_start(&a);
}
// 使用lv_my_anim_fall_down函数添加的对象，禁止修改其userdata
void lv_my_anim_fall_down(lv_obj_t *obj, uint16_t time, uint16_t delay, bool del)
{
    static lv_anim_t a;
    lv_coord_t y_start;
    lv_obj_set_style_transform_pivot_x(obj, 0, 0);
    lv_anim_init(&a);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    lv_anim_set_values(&a, 0, 100);
    lv_anim_set_exec_cb(&a, lv_my_anim_cb_fall_down);
    lv_anim_set_path_cb(&a, lv_anim_path_linear);
    lv_anim_set_var(&a, obj);
    if (del)
    {
        lv_anim_set_ready_cb(&a, [](lv_anim_t* a)
            { lv_obj_del((lv_obj_t*)(a->var)); });
    }
    else
    {
        lv_anim_set_ready_cb(&a, [](lv_anim_t* a)
            {
                lv_obj_t* obj = (lv_obj_t*)(a->var);
                lv_coord_t y_start = (lv_coord_t)((int)lv_obj_get_user_data(obj));
                lv_obj_set_style_opa(obj, 0, 0);
                lv_obj_set_style_transform_angle(obj, 0, 0);
                lv_obj_set_y(obj, y_start);
            });
    }
    y_start = lv_obj_get_style_y(obj, 0);
    lv_obj_set_user_data(obj, reinterpret_cast<void *>(y_start));
    lv_anim_start(&a);
}

void lv_anim_move(lv_obj_t* obj, lv_coord_t x, lv_coord_t y, uint16_t time, uint16_t delay)
{
    lv_anim_t a;
    int16_t p;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    p = lv_obj_get_style_x(obj, 0);
    lv_anim_set_values(&a, p, x);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_path_cb(&a, my_anim_path_ease_in_out);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    lv_anim_start(&a);
    p = lv_obj_get_style_y(obj, 0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&a, p, y);
    lv_anim_start(&a);
}

void lv_anim_size(lv_obj_t* obj, lv_coord_t w, lv_coord_t h, uint16_t time, uint16_t delay)
{
    lv_anim_t a;
    int16_t p;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    p = lv_obj_get_style_width(obj, 0);
    lv_anim_set_values(&a, p, w);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_width);
    lv_anim_set_path_cb(&a, my_anim_path_ease_in_out);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    lv_anim_start(&a);
    p = lv_obj_get_style_height(obj, 0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_height);
    lv_anim_set_values(&a, p, h);
    lv_anim_start(&a);
}
void lv_anim_fly_up(lv_obj_t* obj, lv_coord_t h, uint16_t time, uint16_t delay)
{
    lv_anim_t a;
    int16_t p;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    p = lv_obj_get_style_y(obj, 0);
    lv_anim_set_values(&a, p + h, p);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a, my_anim_path_ease_out);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    lv_anim_start(&a);
    lv_obj_fade_in(obj, time / 2, delay);
}
void lv_anim_fall_fly(lv_obj_t* obj, lv_coord_t h, uint16_t time, uint16_t delay)
{
    lv_anim_t a;
    int16_t p;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    p = lv_obj_get_style_y(obj, 0);
    lv_anim_set_values(&a, p, p + h);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a, my_anim_path_ease_in);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    lv_anim_start(&a);
    lv_obj_fade_out(obj, time / 2, delay + time / 2);
}