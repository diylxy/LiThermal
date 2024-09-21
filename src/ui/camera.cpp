#include <my_main.h>
static void flash_bang_effect()
{
    LOCKLV();
    lv_obj_t *mask_camera = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mask_camera, 320, 240);
    lv_obj_set_style_border_width(mask_camera, 0, 0);
    if (globalSettings.useBlackFlashBang == false)
        lv_obj_set_style_bg_color(mask_camera, lv_color_white(), 0);
    else
        lv_obj_set_style_bg_color(mask_camera, lv_color_black(), 0);
    lv_obj_set_style_radius(mask_camera, 0, 0);
    lv_obj_set_style_opa(mask_camera, 128, 0);
    lv_obj_fade_out(mask_camera, 300, 0);
    lv_obj_del_delayed(mask_camera, 300);
    UNLOCKLV();
}
#define TJE_IMPLEMENTATION
#include "utils/tiny_jpeg.h"
void camera_take_photo_from_stream()
{
    if (cameraUtils.connected == false)
        return;
    if (globalSettings.preserveUI == false)
    {
        cameraUtils.readJpegWithExtra(allocateNewFilename());
    }
    else
    {
        char data[128];
        sprintf(data, "%s.jpeg", allocateNewFilename());
        LOCKLV();
        lv_img_dsc_t *snapshot = lv_snapshot_take(lv_scr_act(), LV_IMG_CF_TRUE_COLOR);
        tje_encode_to_file(data, 320, 240, 4, snapshot->data);
        lv_snapshot_free(snapshot);
        UNLOCKLV();
    }
    flash_bang_effect();
}
#include "lottie_rec.h"
extern bool packet_dumping;
lv_timer_t *tm_create_circle = NULL, *tm_circle = NULL;
lv_obj_t *circle_REC = NULL;
lv_obj_t *lot_rec = NULL;
void camera_record_toggle_dump_stream()
{
    if (cameraUtils.connected == false)
        return;
    if (packet_dumping == true)
    {
        flash_bang_effect();
        LOCKLV();
        if (tm_create_circle)
        {
            lv_timer_del(tm_create_circle);
            tm_create_circle = NULL;
        }
        if (tm_circle)
        {
            lv_timer_del(tm_circle);
            tm_circle = NULL;
        }
        if (lv_obj_is_valid(circle_REC))
        {
            lv_obj_del(circle_REC);
            circle_REC = NULL;
        }
        if (lv_obj_is_valid(lot_rec))
        {
            lv_obj_del(lot_rec);
            lot_rec = NULL;
        }
        UNLOCKLV();
        codec_enablePacketDumping(false, NULL);
    }
    else
    {
        char file_name_buffer[128];
        const char *name_partial = allocateNewFilename();
        if (name_partial == NULL)
        {
            // TODO: 添加错误提示
            return;
        }
        cameraUtils.readJpegWithExtra(name_partial);
        sprintf(file_name_buffer, "%s.mjpeg", name_partial);
        codec_enablePacketDumping(true, file_name_buffer);
        sprintf(file_name_buffer, "%s.raw", name_partial);
        remove(file_name_buffer); // 避免识别成照片
        lot_rec = lv_rlottie_create_from_raw(lv_layer_top(), 200, 200, lottie_rec);
        lv_obj_center(lot_rec);
        lv_rlottie_set_play_mode(lot_rec, LV_RLOTTIE_CTRL_PLAY);
        lv_obj_del_delayed(lot_rec, 5000);

        tm_create_circle = lv_timer_create([](lv_timer_t *tm_out)
                                           {
            circle_REC = lv_obj_create(lv_layer_top());
            lv_obj_set_size(circle_REC, 8, 8);
            lv_obj_align(circle_REC, LV_ALIGN_TOP_RIGHT, -8, 8);
            lv_obj_set_style_radius(circle_REC, LV_RADIUS_CIRCLE, 0);
            lv_obj_set_style_border_width(circle_REC, 0, 0);
            lv_obj_set_style_bg_color(circle_REC, lv_color_hex(0xFF0000), 0);
            lv_obj_fade_in(circle_REC, 700, 0);
            tm_circle = lv_timer_create([](lv_timer_t* tm) {
                if (lv_obj_is_valid(circle_REC) == false)
                {
                    lv_timer_del(tm);
                    circle_REC = NULL;
                    return;
                }
                if (lv_obj_get_style_opa(circle_REC, 0) <= 5)
                {
                    lv_obj_fade_in(circle_REC, 700, 0);
                }
                else
                {
                    lv_obj_fade_out(circle_REC, 700, 0);
                }
                }, 700, NULL);
            lv_timer_del(tm_create_circle);
            tm_create_circle = NULL; }, 3000, 0);
    }
}
