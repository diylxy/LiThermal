/**
 * @file sunxig2d.h
 *
 */

#ifndef SUNXIG2D_H
#define SUNXIG2D_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if USE_SUNXIFB_G2D

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifdef CONF_G2D_VERSION_NEW
#include "g2d_driver_enh.h"
#else
#include "g2d_driver.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef enum {
    SUNXI_G2D_LIMIT_FILL,
    SUNXI_G2D_LIMIT_OPA_FILL,
    SUNXI_G2D_LIMIT_BLIT,
    SUNXI_G2D_LIMIT_BLEND,
    SUNXI_G2D_LIMIT_SCALE
} sunxi_g2d_limit;

/**********************
 * GLOBAL PROTOTYPES
 **********************/
bool sunxifb_g2d_init(uint32_t bits_per_pixel);

void sunxifb_g2d_deinit();

int32_t sunxifb_g2d_get_limit(sunxi_g2d_limit limit);

int sunxifb_g2d_blit_to_fb(uintptr_t src_buf, uint32_t src_w, uint32_t src_h,
        uint32_t src_cx, uint32_t src_cy, uint32_t src_cw, uint32_t src_ch,
        uintptr_t dst_buf, uint32_t dst_w, uint32_t dst_h, uint32_t dst_cx,
        uint32_t dst_cy, uint32_t dst_cw, uint32_t dst_ch,
        g2d_blt_flags_h rotated);

#ifdef LV_USE_SUNXIFB_G2D_FILL
int sunxifb_g2d_fill(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t color, lv_opa_t opa);
#endif /* LV_USE_SUNXIFB_G2D_FILL */

#ifdef LV_USE_SUNXIFB_G2D_BLIT
int sunxifb_g2d_blit(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t *map, const lv_area_t *map_area,
        lv_opa_t opa);
#endif /* LV_USE_SUNXIFB_G2D_BLIT */

#ifdef LV_USE_SUNXIFB_G2D_BLEND
int sunxifb_g2d_blend(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t *map, const lv_area_t *map_area,
        lv_opa_t opa, bool chroma_key);
#endif /* LV_USE_SUNXIFB_G2D_BLEND */

#ifdef LV_USE_SUNXIFB_G2D_SCALE
int sunxifb_g2d_scale(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t *map, const lv_area_t *map_area,
        lv_opa_t opa, uint16_t zoom, const lv_point_t *pivot);
#endif /* LV_USE_SUNXIFB_G2D_SCALE */

/**********************
 *      MACROS
 **********************/

#endif  /*USE_SUNXIFB_G2D*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*SUNXIG2D_H*/
