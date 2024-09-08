#include <my_main.h>

static lv_obj_t *image_obj[5];
static lv_color_t canvas_buffer[5][240 * 180];
static int image_src_id[5];
static lv_draw_img_dsc_t canvas_draw_dsc;
static int image_pointer_left = 0; // 左边第一个的位置
static int image_spare_left = 0;   // 左边空闲出的控件个数
static const int image_pos_x_lut[4] = {20, 25, 35, 50};
static const int image_pos_y_lut[4] = {12, 8, 4, 0};
static const int image_fly_delay_lut[4] = {150, 100, 50, 0};
#define GALLERY_POS_RIGHT_X 400
static void opa_anim_cb(void *obj, int32_t v)
{
    lv_obj_set_style_opa((lv_obj_t *)obj, v, 0);
}

static void lv_anim_opa(lv_obj_t *obj, lv_opa_t opa, uint16_t time, uint16_t delay)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, lv_obj_get_style_opa(obj, 0), opa);
    lv_anim_set_exec_cb(&a, opa_anim_cb);
    lv_anim_set_time(&a, time);
    lv_anim_set_delay(&a, delay);
    lv_anim_start(&a);
}

static void image_obj_create()
{
    canvas_draw_dsc.angle = 0;
    image_pointer_left = image_spare_left;
    for (int i = 0; i < 4; ++i)
    {
        image_obj[i] = lv_obj_create(lv_layer_top());
        lv_obj_clear_flag(image_obj[i], LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_size(image_obj[i], 240, 180);
        lv_obj_align(image_obj[i], LV_ALIGN_LEFT_MID, image_pos_x_lut[i], image_pos_y_lut[i]);
        lv_obj_set_style_opa(image_obj[i], 0, 0);
        lv_obj_set_style_clip_corner(image_obj[i], true, 0);
        lv_obj_t *canvas_img = lv_canvas_create(image_obj[i]);
        lv_canvas_set_buffer(canvas_img, canvas_buffer[i], 240, 180, LV_IMG_CF_TRUE_COLOR);
        lv_obj_center(canvas_img);
        if (image_spare_left <= i)
            lv_anim_fly_up(image_obj[i], MY_FLY_ANIM_DEFAULT_HEIGHT, MY_FLY_UP_ANIM_DEFAULT_TIME, image_fly_delay_lut[i]);
    }
    image_obj[4] = lv_obj_create(lv_layer_top());
    lv_obj_clear_flag(image_obj[4], LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(image_obj[4], 240, 180);
    lv_obj_align(image_obj[4], LV_ALIGN_LEFT_MID, GALLERY_POS_RIGHT_X, 0);
    lv_obj_set_style_opa(image_obj[4], 0, 0);
    lv_obj_set_style_clip_corner(image_obj[4], true, 0);
    lv_obj_t *canvas_img = lv_canvas_create(image_obj[4]);
    lv_canvas_set_buffer(canvas_img, canvas_buffer[4], 240, 180, LV_IMG_CF_TRUE_COLOR);
    lv_obj_center(canvas_img);
}

static void image_obj_slide_left()
{
    int spare_obj = (image_pointer_left + 4 - image_spare_left) % 5;
    if (image_spare_left >= 3)
        return;
    // 处理空闲的obj
    lv_obj_set_pos(image_obj[spare_obj], GALLERY_POS_RIGHT_X, 0);
    lv_obj_set_style_opa(image_obj[spare_obj], LV_OPA_COVER, 0);
    lv_obj_move_foreground(image_obj[spare_obj]);
    lv_anim_del(image_obj[spare_obj], NULL);
    lv_anim_move(image_obj[spare_obj], image_pos_x_lut[3], 0, MY_MOVE_ANIM_DEFAULT_TIME, 0);

    for (int i = 1; i < 4 - image_spare_left; ++i)
    {
        lv_anim_move(image_obj[(image_pointer_left + i) % 5], image_pos_x_lut[i - 1 + image_spare_left], image_pos_y_lut[i - 1 + image_spare_left], MY_MOVE_ANIM_DEFAULT_TIME, image_fly_delay_lut[i]);
    }
    lv_anim_del(image_obj[image_pointer_left], NULL);
    lv_obj_fade_out(image_obj[image_pointer_left], MY_MOVE_ANIM_DEFAULT_TIME, 0);
    image_pointer_left = (image_pointer_left + 1) % 5;
}

static void image_obj_slide_right()
{
    int spare_obj = (image_pointer_left + 4) % 5;
    if (image_spare_left >= 3)
        return;
    // 处理空闲的obj
    lv_obj_set_style_opa(image_obj[spare_obj], 0, 0);
    lv_obj_set_pos(image_obj[spare_obj], image_pos_x_lut[image_spare_left], image_pos_y_lut[image_spare_left]);
    lv_obj_move_background(image_obj[spare_obj]);
    lv_anim_del(image_obj[spare_obj], NULL);
    lv_obj_fade_in(image_obj[spare_obj], MY_MOVE_ANIM_DEFAULT_TIME, 0);

    for (int i = 0; i < 3 - image_spare_left; ++i)
    {
        lv_anim_move(image_obj[(image_pointer_left + i) % 5], image_pos_x_lut[i + 1 + image_spare_left], image_pos_y_lut[i + 1 + image_spare_left], MY_MOVE_ANIM_DEFAULT_TIME, image_fly_delay_lut[i + image_spare_left]);
    }
    lv_anim_move(image_obj[(image_pointer_left + 3 - image_spare_left) % 5], GALLERY_POS_RIGHT_X, 0, MY_MOVE_ANIM_DEFAULT_TIME, 0);
    image_pointer_left = spare_obj;
}

// 相当于右滑，但当前显示的照片动画不同
static void image_obj_del_current()
{
    int spare_obj = (image_pointer_left + 4) % 5;
    if (image_spare_left >= 3)
        return;
    // 处理空闲的obj
    lv_obj_set_style_opa(image_obj[spare_obj], 0, 0);
    lv_obj_set_pos(image_obj[spare_obj], image_pos_x_lut[image_spare_left], image_pos_y_lut[image_spare_left]);
    lv_obj_move_background(image_obj[spare_obj]);
    lv_anim_del(image_obj[spare_obj], NULL);
    lv_obj_fade_in(image_obj[spare_obj], MY_MOVE_ANIM_DEFAULT_TIME, 0);

    for (int i = 0; i < 3 - image_spare_left; ++i)
    {
        lv_anim_move(image_obj[(image_pointer_left + i) % 5], image_pos_x_lut[i + 1 + image_spare_left], 0, MY_MOVE_ANIM_DEFAULT_TIME, image_fly_delay_lut[i + image_spare_left]);
    }
    lv_anim_del(image_obj[(image_pointer_left + 3 - image_spare_left) % 5], NULL);
    lv_my_anim_fall_down(image_obj[(image_pointer_left + 3 - image_spare_left) % 5], MY_POP_FALL_ANIM_DEFAULT_TIME, 0, false);
    image_pointer_left = spare_obj;
}

static void image_obj_close()
{
    for (int i = 0; i < 4 - image_spare_left; ++i)
    {
        lv_anim_fall_fly(image_obj[(image_pointer_left + i) % 5], MY_FLY_ANIM_DEFAULT_HEIGHT, MY_FLY_DOWN_ANIM_DEFAULT_TIME, image_fly_delay_lut[i + image_spare_left]);
        lv_obj_del_delayed(image_obj[(image_pointer_left + i) % 5], MY_FLY_DOWN_ANIM_DEFAULT_TIME + image_fly_delay_lut[i + image_spare_left]);
    }
    lv_obj_del(image_obj[(image_pointer_left + 4 - image_spare_left) % 5]);
}

/////////////////////////////////////////////// 图像列表相关
#include <stdio.h>
#include <string.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dirent.h>
#include <stdint.h>
#include <sys/types.h>
#define GALLERY_MAX_IMAGES 100000 // 必须为8的倍数
static uint8_t image_hashmap[GALLERY_MAX_IMAGES / 8];
static int totalImages = 0;
static int centerImageID = 0;       // 当前屏幕中间显示的图像在数组中的位置
static const uint32_t countTable[256] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};

int getTotalImages()
{
    int total = 0;
    for (int i = 0; i < sizeof(image_hashmap); ++i)
    {
        total += countTable[image_hashmap[i]];
    }
    return total;
}

int getLastImageID()
{
    int i = 0;
    int j = 0;
    for (i = sizeof(image_hashmap) - 1; i >= 0; --i)
    {
        if (image_hashmap[i] != 0)
            break;
    }
    if (i < 0)
        return 0;
    if (image_hashmap[i] == 0xff)
    {
        if (i == sizeof(image_hashmap) - 1)
            return -1;
        return (i + 1) * 8;
    }
    for (j = 7; j >= 0; --j)
    {
        if ((1 << j) & image_hashmap[i])
        {
            return i * 8 + j + 1;
        }
    }
    return i * 8;
}

int getNextImage(int current)
{
    int result;
    int i, j;
    if (current < 0)
        current = -1;
    current += 1;
    i = current / 8;
    j = current % 8;
    for (; i < sizeof(image_hashmap); ++i)
    {
        for (; j < 8; ++j)
        {
            if ((1 << j) & image_hashmap[i])
            {
                return i * 8 + j;
            }
        }
        j = 0;
    }
    if (current == 0)
        return -1;
    else
        return getNextImage(-1);
}

int getPrevImage(int current)
{
    int result;
    int i, j;
    if (current <= 0)
        current = GALLERY_MAX_IMAGES;
    current -= 1;
    i = current / 8;
    j = current % 8;
    for (; i >= 0; --i)
    {
        for (; j >= 0; --j)
        {
            if ((1 << j) & image_hashmap[i])
            {
                return i * 8 + j;
            }
        }
        j = 7;
    }
    if (current == GALLERY_MAX_IMAGES - 1)
        return -1;
    else
        return getPrevImage(GALLERY_MAX_IMAGES);
}

void readFiles(const char *dirToOpen)
{
    DIR *dir;
    struct dirent *entry;
    int id;
    static char ext[10];
    if (!(dir = opendir(dirToOpen)))
        return;
    if (!(entry = readdir(dir)))
        return;
    do
    {
        if (entry->d_type != DT_DIR)
        {
            // 避免缓冲区溢出
            int len = strlen(entry->d_name);
            int ext_pos = strchr(entry->d_name, '.') - entry->d_name;
            if (ext_pos - len > 6)
                continue;
            int param_result = sscanf(entry->d_name, "CAP%d.%s", &id, ext);
            if (param_result != 2)
                continue;
            image_hashmap[id / 8] |= 1 << (id % 8);
        }
    } while (entry = readdir(dir));
    closedir(dir);
    totalImages = getTotalImages();
}

/////////////////////////////////////////////// 相册功能及相关状态
typedef enum
{
    GALLERY_STATE_LIST,
    GALLERY_STATE_FULLSCREEN,
    GALLERY_STATE_MENU,
} gallery_state_t;
#define GALLERY_CARD_SHOW_Y -13
#define GALLERY_CARD_HIDE_Y -43
gallery_state_t current_state = GALLERY_STATE_LIST;
MyCard card_gallery;
lv_obj_t *lbl_fileid = NULL;
void menu_gallery_show()
{
    readFiles(GALLERY_PATH);
    image_obj_create();
    card_gallery.create(lv_layer_top(), 0, GALLERY_CARD_HIDE_Y, 120, 33, LV_ALIGN_TOP_MID);
    card_gallery.show(CARD_ANIM_NONE);
    card_gallery.move(0, GALLERY_CARD_SHOW_Y);
    lv_obj_clear_flag(card_gallery.obj, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_opa(card_gallery.obj, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(card_gallery.obj, 0, 0);
    lbl_fileid = lv_label_create(card_gallery.obj);
    lv_obj_set_align(lbl_fileid, LV_ALIGN_TOP_MID);
    lv_label_set_text_fmt(lbl_fileid, "%d/%d", centerImageID, totalImages);
    current_state = GALLERY_STATE_LIST;
    HAL::key_press_event[2] = false; // 菜单按钮事件
    HAL::key_press_event[3] = false; // 全屏切换按钮事件
}

static void menu_gallery_hide()
{
    image_obj_close();
    card_gallery.move(0, GALLERY_CARD_HIDE_Y);
    lv_obj_del_delayed(card_gallery.obj, MY_MOVE_ANIM_DEFAULT_TIME);
    card_gallery.obj = NULL;
    lbl_fileid = NULL;
}

void menu_gallery_loop(bool has_hal_go_back_event)
{
    if (has_hal_go_back_event)
    {
        LOCKLV();
        menu_gallery_hide();
        current_mode = MODE_MAINPAGE;
        UNLOCKLV();
    }
    switch (current_state)
    {
    case GALLERY_STATE_LIST:
        if (last_encoder_direction > 0)
        {
            last_encoder_direction = 0;
            LOCKLV();
            image_obj_slide_right();
            UNLOCKLV();
        }
        else if (last_encoder_direction < 0)
        {
            last_encoder_direction = 0;
            LOCKLV();
            image_obj_slide_left();
            UNLOCKLV();
        }
        if (HAL::key_press_event[2])
        {
            HAL::key_press_event[2] = false;
            printf("Gallery menu requested\n");
        }
        if (HAL::key_press_event[3])
        {
            HAL::key_press_event[3] = false;
            printf("Gallery fullscreen requested\n");
        }
        break;

    default:
        break;
    }
}