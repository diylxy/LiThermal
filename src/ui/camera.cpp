#include <my_main.h>

void camera_take_photo_from_stream()
{
    LOCKLV();
    lv_obj_t *mask_camera = lv_obj_create(lv_scr_act());
    lv_obj_set_size(mask_camera, 320, 240);
    lv_obj_set_style_border_width(mask_camera, 0, 0);
    lv_obj_set_style_bg_color(mask_camera, lv_color_white(), 0);
    lv_obj_set_style_radius(mask_camera, 0, 0);
    lv_obj_set_style_opa(mask_camera, 128, 0);
    lv_obj_fade_out(mask_camera, 300, 0);
    lv_obj_del_delayed(mask_camera, 300);
    UNLOCKLV();
    cameraUtils.readJpegWithExtra(allocateNewFilename());
}
