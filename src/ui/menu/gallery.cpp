#include <my_main.h>

static lv_obj_t *image_obj[5];
static lv_color_t canvas_buffer[5][240 * 180];
static lv_color_t canvas_fullscreen_buffer[320 * 240];
static int image_src_id[5];
static lv_draw_img_dsc_t canvas_draw_dsc;
lv_obj_t *lbl_fileid = NULL;
static int totalImages = 0;
static int centerImageID = 0; // 当前屏幕中间显示的图像在数组中的位置
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

typedef enum
{
    PHOTO_TYPE_FILE_NOT_FOUND,
    PHOTO_TYPE_RAW_CAPTURE,
    PHOTO_TYPE_SCREENSHOT,
    PHOTO_TYPE_VIDEO,
} photo_type_t;

static photo_type_t getPhotoType(int id)
{
    char filename_buffer[128];
    struct stat s;
    sprintf(filename_buffer, GALLERY_PATH "/CAP%05d.jpeg", id);
    if (stat(filename_buffer, &s) != 0)
        return PHOTO_TYPE_FILE_NOT_FOUND;
    sprintf(filename_buffer, GALLERY_PATH "/CAP%05d.raw", id);
    if (stat(filename_buffer, &s) == 0)
        return PHOTO_TYPE_RAW_CAPTURE;
    sprintf(filename_buffer, GALLERY_PATH "/CAP%05d.mjpeg", id);
    if (stat(filename_buffer, &s) == 0)
        return PHOTO_TYPE_VIDEO;
    return PHOTO_TYPE_SCREENSHOT;
}

/// @brief 渲染图像/视频缩略图到canvas
/// @param obj_id 对应canvas在image_obj中的位置
static void image_obj_render(int obj_id)
{
    char filename_buffer[128];
    if (image_src_id[obj_id] < 0)
        return;
    sprintf(filename_buffer, "/mnt/UDISK/DCIM/CAP%05d.jpeg", image_src_id[obj_id]);
    lv_obj_t *obj_canvas = lv_obj_get_child(image_obj[obj_id], 0);
    switch (getPhotoType(image_src_id[obj_id]))
    {
    case PHOTO_TYPE_RAW_CAPTURE:
    case PHOTO_TYPE_VIDEO:
        canvas_draw_dsc.zoom = 128 * 3;
        lv_canvas_draw_img(obj_canvas, 0, 0, filename_buffer, &canvas_draw_dsc);
        break;
    default:
        printf("SKIP\n");
        break;
    }
}

static void image_obj_create()
{
    canvas_draw_dsc.angle = 0;
    canvas_draw_dsc.antialias = 1;
    canvas_draw_dsc.blend_mode = LV_BLEND_MODE_NORMAL;
    canvas_draw_dsc.frame_id = 0;
    canvas_draw_dsc.opa = LV_OPA_COVER;
    canvas_draw_dsc.pivot.x = 0;
    canvas_draw_dsc.pivot.y = 0;
    canvas_draw_dsc.recolor = lv_color_black();
    canvas_draw_dsc.recolor_opa = 0;
    canvas_draw_dsc.zoom = 128 * 3;
    image_pointer_left = image_spare_left;
    LOCKLV();
    memset(image_src_id, 0xff, sizeof(image_src_id));
    for (int i = 3; i >= 0; --i)
    {
        if (i == 3)
            image_src_id[i] = getPrevImage(-1);
        else if (i >= image_spare_left)
            image_src_id[i] = getPrevImage(image_src_id[i + 1]);
        else
            image_src_id[i] = -1;
    }
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
        else
        {
            image_src_id[i] = -1;
        }
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
    for (int i = 0; i < 5; ++i)
    {
        image_obj_render(i);
    }
    UNLOCKLV();
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
    image_src_id[spare_obj] = getNextImage(image_src_id[(image_pointer_left + 3 - image_spare_left) % 5]);
    image_obj_render(spare_obj);
    if (image_src_id[(spare_obj + 4) % 5] > image_src_id[spare_obj])
        centerImageID = 1;
    else
        centerImageID += 1;
    lv_label_set_text_fmt(lbl_fileid, "%d/%d", centerImageID, totalImages);

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
    image_src_id[spare_obj] = getPrevImage(image_src_id[image_pointer_left]);
    image_obj_render(spare_obj);
    if (image_src_id[(image_pointer_left + (2 - image_spare_left) + 5) % 5] > image_src_id[(image_pointer_left + (3 - image_spare_left) + 5) % 5])
        centerImageID = totalImages;
    else
        centerImageID -= 1;
    lv_label_set_text_fmt(lbl_fileid, "%d/%d", centerImageID, totalImages);

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
/////////////////////////////////////////////// 相册全屏显示控件
lv_obj_t *ffmpeg_fullscreen = NULL;
static void full_screen_show(int id)
{
    photo_type_t type = getPhotoType(id);
    char file_name_buffer[128];

    if (type == PHOTO_TYPE_FILE_NOT_FOUND)
    {
        current_state = GALLERY_STATE_LIST;
        return;
    }
    if (ffmpeg_fullscreen != NULL)
        lv_obj_del(ffmpeg_fullscreen);
    if (type == PHOTO_TYPE_VIDEO)
    {
        sprintf(file_name_buffer, GALLERY_PATH "/CAP%05d.mjpeg", id);
        ffmpeg_fullscreen = lv_ffmpeg_player_create(lv_layer_top());
        lv_ffmpeg_player_set_src(ffmpeg_fullscreen, file_name_buffer);
        lv_ffmpeg_player_set_cmd(ffmpeg_fullscreen, LV_FFMPEG_PLAYER_CMD_START);
        lv_ffmpeg_player_set_auto_restart(ffmpeg_fullscreen, true);
        lv_obj_fade_in(ffmpeg_fullscreen, 500, 750);
    }
    else
    {
        sprintf(file_name_buffer, GALLERY_PATH "/CAP%05d.jpeg", id);
        ffmpeg_fullscreen = lv_canvas_create(lv_layer_top());
        lv_canvas_set_buffer(ffmpeg_fullscreen, canvas_fullscreen_buffer, 320, 240, LV_IMG_CF_TRUE_COLOR);
        if (type == PHOTO_TYPE_RAW_CAPTURE)
            canvas_draw_dsc.zoom = 256 * 2;
        lv_canvas_draw_img(ffmpeg_fullscreen, 0, 0, file_name_buffer, &canvas_draw_dsc);
        lv_obj_fade_in(ffmpeg_fullscreen, 500, 0);
    }
    lv_obj_center(ffmpeg_fullscreen);
    current_state = GALLERY_STATE_FULLSCREEN;
}

static void full_screen_hide()
{
    if (ffmpeg_fullscreen == NULL)
        return;
    lv_obj_fade_out(ffmpeg_fullscreen, 500, 0);
    lv_obj_del_delayed(ffmpeg_fullscreen, 500);
    ffmpeg_fullscreen = NULL;
    current_state = GALLERY_STATE_LIST;
}
/////////////////////////////////////////////// 相册功能及相关状态
void menu_gallery_show()
{
    readFiles(GALLERY_PATH);
    totalImages = getTotalImages();
    centerImageID = totalImages;
    if (totalImages == 0)
    {
        current_mode = MODE_MAINPAGE;
        return;
    }
    if (totalImages < 4)
    {
        image_spare_left = 4 - totalImages;
    }
    else
    {
        image_spare_left = 0;
    }
    image_obj_create();
    LOCKLV();
    card_gallery.create(lv_layer_top(), 0, GALLERY_CARD_HIDE_Y, 120, 33, LV_ALIGN_TOP_MID);
    card_gallery.show(CARD_ANIM_NONE);
    card_gallery.move(0, GALLERY_CARD_SHOW_Y);
    lv_obj_clear_flag(card_gallery.obj, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_opa(card_gallery.obj, 128, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(card_gallery.obj, 0, 0);
    lbl_fileid = lv_label_create(card_gallery.obj);
    lv_obj_set_align(lbl_fileid, LV_ALIGN_TOP_MID);
    lv_label_set_text_fmt(lbl_fileid, "%d/%d", centerImageID, totalImages);
    UNLOCKLV();
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
        if (current_state == GALLERY_STATE_LIST)
        {
            LOCKLV();
            menu_gallery_hide();
            current_mode = MODE_MAINPAGE;
            UNLOCKLV();
        }
    }
    switch (current_state)
    {
    case GALLERY_STATE_LIST:
        if (last_encoder_direction > 0)
        {
            LOCKLV();
            image_obj_slide_right();
            UNLOCKLV();
            last_encoder_direction = 0;
        }
        else if (last_encoder_direction < 0)
        {
            LOCKLV();
            image_obj_slide_left();
            UNLOCKLV();
            last_encoder_direction = 0;
        }
        if (HAL::key_press_event[2])
        {
            HAL::key_press_event[2] = false;
            printf("Gallery menu requested\n");
        }
        if (HAL::key_press_event[3])
        {
            HAL::key_press_event[3] = false;
            LOCKLV();
            full_screen_show(image_src_id[(image_pointer_left + 3) % 5]);
            UNLOCKLV();
        }
        break;
    case GALLERY_STATE_FULLSCREEN:
        if (last_encoder_direction > 0)
        {
            // TODO: FastForward
            last_encoder_direction = 0;
        }
        else if (last_encoder_direction < 0)
        {
            // TODO: Rewind
            last_encoder_direction = 0;
        }
        HAL::key_press_event[2] = false;
        if (HAL::key_press_event[3])
        {
            HAL::key_press_event[3] = false;
            LOCKLV();
            full_screen_hide();
            UNLOCKLV();
        }
    default:
        break;
    }
}