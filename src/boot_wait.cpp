#include "my_main.h"
extern "C" const lv_img_dsc_t ui_img_loading_png;
extern "C" const lv_font_t ui_font_ali36;
#define STAGE_LOAD_NCM_PCT_START 0
#define STAGE_LOAD_NCM_PCT_STEP 20
#define STAGE_LOAD_HTTP_PCT_START 350
#define STAGE_LOAD_HTTP_PCT_STEP 20
#define STAGE_LOAD_BAR_TOTAL 860
static int current_bar_pos = STAGE_LOAD_NCM_PCT_START;
#define STAGE_FIRST 0
#define STAGE_WAIT_NCM 1
#define STAGE_WAIT_HTTP 2
#define STAGE_FIN 3
static int current_stage = STAGE_FIRST;
static lv_obj_t *bar_boot;
static lv_obj_t *lbl_boot_info;

void waitboot_scr_load(void)
{
    lv_obj_t *ui_Image1 = lv_img_create(lv_scr_act());
    lv_img_set_src(ui_Image1, &ui_img_loading_png);
    lv_obj_set_width(ui_Image1, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Image1, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Image1, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_Image1, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_Image1, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_my_anim_fall_down(ui_Image1, MY_POP_FALL_ANIM_DEFAULT_TIME, 100);
    if(HAL::checkCameraIF() == true)
    {
        HAL::setCameraIFIP();
        videoPlayer.init();
        videoPlayer.connect();
        return;
    }

    bar_boot = lv_bar_create(lv_scr_act());
    lv_bar_set_range(bar_boot, 0, STAGE_LOAD_BAR_TOTAL);
    lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_OFF);
    lv_obj_set_width(bar_boot, 220);
    lv_obj_set_height(bar_boot, 10);
    lv_obj_set_x(bar_boot, 0);
    lv_obj_set_y(bar_boot, 10);
    lv_obj_set_align(bar_boot, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(bar_boot, 0, 0);
    lv_obj_set_style_anim_time(bar_boot, 500, 0);

    lbl_boot_info = lv_label_create(lv_scr_act());
    lv_obj_set_width(lbl_boot_info, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(lbl_boot_info, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(lbl_boot_info, 0);
    lv_obj_set_y(lbl_boot_info, -26);
    lv_obj_set_align(lbl_boot_info, LV_ALIGN_CENTER);
    lv_obj_set_style_text_font(lbl_boot_info, &ui_font_ali36, LV_PART_MAIN);
    lv_label_set_text(lbl_boot_info, "等待NCM...");
    lv_obj_fade_in(lbl_boot_info, 500, 0);
    lv_timer_create([](lv_timer_t *t)
                    {
        switch(current_stage)
        {
            case STAGE_FIRST:
                lv_anim_fly_up(bar_boot);
                current_stage = STAGE_WAIT_NCM;
                current_bar_pos = STAGE_LOAD_NCM_PCT_START;
                break;
            case STAGE_WAIT_NCM:
                if(HAL::checkCameraIF() == true)
                {
                    current_stage = STAGE_WAIT_HTTP;
                    current_bar_pos = STAGE_LOAD_HTTP_PCT_START;
                    HAL::setCameraIFIP();
                    lv_label_set_text(lbl_boot_info, "等待HTTP...");
                    lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_ON);
                    break;
                }
                if(current_bar_pos < STAGE_LOAD_HTTP_PCT_START)
                    current_bar_pos += STAGE_LOAD_NCM_PCT_STEP;
                lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_ON);
                break;
            case STAGE_WAIT_HTTP:
                if(current_bar_pos < STAGE_LOAD_BAR_TOTAL)
                    current_bar_pos += STAGE_LOAD_HTTP_PCT_STEP;
                else
                {
                    videoPlayer.init();
                    videoPlayer.connect();
                    current_stage = STAGE_FIN;
                }
                lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_ON);
                break;
            case STAGE_FIN:
                lv_label_set_text(lbl_boot_info, "请求RTSP...");
                current_bar_pos = STAGE_LOAD_BAR_TOTAL;
                lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_ON);
                lv_my_anim_fall_down(lbl_boot_info, MY_POP_FALL_ANIM_DEFAULT_TIME, 1800, true);
                lv_my_anim_fall_down(bar_boot, MY_POP_FALL_ANIM_DEFAULT_TIME, 2000, true);
                lv_timer_del(t);
                break;
            default:
                lv_timer_del(t);
                break;

        } }, 500, NULL);
}
