/**
 * @file sunxifb.h
 *
 */

#ifndef SUNXIFB_H
#define SUNXIFB_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "../../lv_drv_conf.h"
#endif
#endif

#if USE_SUNXIFB

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void sunxifb_init(uint32_t rotated);
void sunxifb_exit(void);
void sunxifb_flush(lv_disp_drv_t * drv, const lv_area_t * area, lv_color_t * color_p);
void sunxifb_get_sizes(uint32_t *width, uint32_t *height);
void* sunxifb_alloc(size_t size, char *label);
void sunxifb_free(void **data, char *label);
#ifdef USE_SUNXIFB_DOUBLE_BUFFER
bool sunxifb_get_dbuf_en();
int sunxifb_set_dbuf_en(lv_disp_drv_t * drv, bool dbuf_en);
#endif /* USE_SUNXIFB_DOUBLE_BUFFER */

/**********************
 *      MACROS
 **********************/

#endif  /*USE_SUNXIFB*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*SUNXIFB_H*/
