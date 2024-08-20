#include "my_main.h"
extern "C" const lv_img_dsc_t ui_img_loading_png;
extern "C" const lv_font_t ui_font_ali36;
#define STAGE_LOAD_NCM_PCT_START 0
#define STAGE_LOAD_NCM_PCT_STEP 20
#define STAGE_LOAD_HTTP_PCT_START 380
#define STAGE_LOAD_HTTP_PCT_STEP 20
#define STAGE_LOAD_BAR_TOTAL 880
static int current_bar_pos = STAGE_LOAD_NCM_PCT_START;
#define STAGE_FIRST 0
#define STAGE_WAIT_NCM 1
#define STAGE_WAIT_HTTP 2
#define STAGE_FIN 3
static int current_stage = STAGE_FIRST;
static lv_obj_t *bar_boot;
static lv_obj_t *rlottie_boot;
static lv_rlottie_t *rlottie_boot_obj;
static const char *next_anim_json = NULL;
#include "lottie_jsons.h"
#include <rlottie_capi.h>
void waitboot_scr_load(void)
{
    lv_obj_t *ui_Image1 = lv_img_create(lv_scr_act());
    lv_img_set_src(ui_Image1, &ui_img_loading_png);
    lv_obj_set_width(ui_Image1, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_Image1, LV_SIZE_CONTENT); /// 1
    lv_obj_set_align(ui_Image1, LV_ALIGN_CENTER);
    if (cameraUtils.checkCameraConnection() == true)
    {
        lv_obj_fade_out(ui_Image1, 500, 0);
        lv_obj_del_delayed(ui_Image1, 500);
        cameraUtils.setCameraIP();
        videoPlayer.init();
        videoPlayer.connect();
        return;
    }

    lv_obj_del_delayed(ui_Image1, 40);

    bar_boot = lv_bar_create(lv_scr_act());
    lv_bar_set_range(bar_boot, 0, STAGE_LOAD_BAR_TOTAL);
    lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_OFF);
    lv_obj_set_width(bar_boot, 220);
    lv_obj_set_height(bar_boot, 5);
    lv_obj_set_x(bar_boot, 0);
    lv_obj_set_y(bar_boot, 70);
    lv_obj_set_align(bar_boot, LV_ALIGN_CENTER);
    lv_obj_set_style_opa(bar_boot, 0, 0);
    lv_obj_set_style_anim_time(bar_boot, 500, 0);

    rlottie_boot = lv_rlottie_create_from_raw(lv_scr_act(), 260, 120, lottie_boot_to_stage1_json);
    next_anim_json = lottie_stage1_json;
    lv_rlottie_set_play_mode(rlottie_boot, (lv_rlottie_ctrl_t)(LV_RLOTTIE_CTRL_PLAY));
    lv_rlottie_set_current_frame(rlottie_boot, 0);
    rlottie_boot_obj = (lv_rlottie_t *)rlottie_boot;
    lv_obj_add_event_cb(rlottie_boot, [](lv_event_t *event)
                        {
                            if (rlottie_boot == NULL)
                                return;
                            if (next_anim_json != NULL)
                            {
                                if (rlottie_boot_obj->animation)
                                {
                                    lottie_animation_destroy(rlottie_boot_obj->animation);
                                    rlottie_boot_obj->animation = lottie_animation_from_data(next_anim_json, next_anim_json, "");
                                    rlottie_boot_obj->current_frame = 0;
                                }
                                if (next_anim_json == lottie_stage1_to_2_json)
                                {
                                    next_anim_json = lottie_stage2_json;
                                    rlottie_boot_obj->play_ctrl = (lv_rlottie_ctrl_t)(LV_RLOTTIE_CTRL_PLAY);
                                    lv_timer_resume(rlottie_boot_obj->task);
                                }
                                else if (next_anim_json == lottie_connecting_json)
                                {
                                    next_anim_json = NULL;
                                    rlottie_boot_obj->play_ctrl = (lv_rlottie_ctrl_t)(LV_RLOTTIE_CTRL_PLAY);
                                    lv_timer_resume(rlottie_boot_obj->task);
                                }
                                else
                                {
                                    next_anim_json = NULL;
                                    rlottie_boot_obj->play_ctrl = (lv_rlottie_ctrl_t)(LV_RLOTTIE_CTRL_PLAY | LV_RLOTTIE_CTRL_LOOP);
                                    lv_timer_resume(rlottie_boot_obj->task);
                                }
                            } }, LV_EVENT_READY, NULL);
    lv_obj_center(rlottie_boot);
    lv_obj_set_y(rlottie_boot, -10);
    lv_timer_create([](lv_timer_t *t)
                    {
        switch(current_stage)
        {
            case STAGE_FIRST:
                lv_obj_fade_in(bar_boot, 300, 0);
                current_stage = STAGE_WAIT_NCM;
                current_bar_pos = STAGE_LOAD_NCM_PCT_START;
                break;
            case STAGE_WAIT_NCM:
                if(cameraUtils.checkCameraConnection() == true)
                {
                    current_stage = STAGE_WAIT_HTTP;
                    current_bar_pos = STAGE_LOAD_HTTP_PCT_START;
                    cameraUtils.setCameraIP();
                    lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_ON);
                    next_anim_json = lottie_stage1_to_2_json;
                    lv_rlottie_set_play_mode(rlottie_boot, (lv_rlottie_ctrl_t)(LV_RLOTTIE_CTRL_PLAY));
                    break;
                }
                if(current_bar_pos < STAGE_LOAD_HTTP_PCT_START)
                    current_bar_pos += STAGE_LOAD_NCM_PCT_STEP;
                lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_ON);
                break;
            case STAGE_WAIT_HTTP:
                if (current_bar_pos == STAGE_LOAD_BAR_TOTAL - STAGE_LOAD_HTTP_PCT_STEP * 2)
                {
                    next_anim_json = lottie_connecting_json;
                    lv_rlottie_set_play_mode(rlottie_boot, (lv_rlottie_ctrl_t)(LV_RLOTTIE_CTRL_PLAY));
                }
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
                current_bar_pos = STAGE_LOAD_BAR_TOTAL;
                lv_bar_set_value(bar_boot, current_bar_pos, LV_ANIM_ON);
                lv_obj_del_delayed(rlottie_boot, 4000);
                lv_obj_del_delayed(bar_boot, 4000);
                lv_timer_del(t);
                break;
            default:
                lv_timer_del(t);
                break;

        } }, 500, NULL);
}
