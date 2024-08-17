/**
 * @file sunxig2d.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "sunxig2d.h"

#if USE_SUNXIFB_G2D

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "sunximem.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      STRUCTURES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/
static int g_g2dfd;
static g2d_fmt_enh g_format;

#ifdef LV_USE_SUNXIFB_G2D_BLEND
static uint32_t color_key;
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/
bool sunxifb_g2d_init(uint32_t bits_per_pixel) {
    if (g_g2dfd > 0)
        return true;

    if ((g_g2dfd = open("/dev/g2d", O_RDWR)) < 0) {
        perror("Error: cannot open g2d device");
        return false;
    }

    switch (bits_per_pixel) {
    case 24:
        g_format = G2D_FORMAT_RGB888;
        break;
    case 16:
        g_format = G2D_FORMAT_RGB565;
        break;
    default:
        g_format = G2D_FORMAT_ARGB8888;
        break;
    }

#ifdef LV_USE_SUNXIFB_G2D_FILL
    printf("Turn on 2d hardware acceleration fill.\n");
#endif

#ifdef LV_USE_SUNXIFB_G2D_BLIT
    printf("Turn on 2d hardware acceleration blit.\n");
#endif

#ifdef LV_USE_SUNXIFB_G2D_BLEND
    printf("Turn on 2d hardware acceleration blend.\n");
    color_key = lv_color_to32(LV_COLOR_CHROMA_KEY);
#endif

#ifdef LV_USE_SUNXIFB_G2D_SCALE
    printf("Turn on 2d hardware acceleration scale.\n");
#endif

    return true;
}

void sunxifb_g2d_deinit() {
    if (g_g2dfd > 0) {
        close(g_g2dfd);
        g_g2dfd = 0;
    }
}

/**
 * 1. For 32bpp formats such as ARGB8888, the output width is required to be greater than 2
 * 2. For 24bpp formats such as RGB888, the output width is required to be greater than 3
 * 3. For 16bpp formats such as RGB565, the output width is required to be greater than 4
 */
int32_t sunxifb_g2d_get_limit(sunxi_g2d_limit limit) {
    /* The following data is tested when the cpu frequency is 1.2GHz */
    switch (limit) {
    case SUNXI_G2D_LIMIT_FILL:
        /* In the case of buffer with cache, it will never be as fast as cpu */
        return 2073600;
    case SUNXI_G2D_LIMIT_OPA_FILL:
        /* 110x110=12100, g2d is 311us~357us, cpu is 338us~669us */
        return 12100;
    case SUNXI_G2D_LIMIT_BLIT:
        /* 220x220=48400, g2d is 373us~577us, cpu is 438us~751us */
        return 48400;
    case SUNXI_G2D_LIMIT_BLEND:
        /* 100x100=10000, g2d is 176us~326us, cpu is 196us~400us */
        return 10000;
    case SUNXI_G2D_LIMIT_SCALE:
        /* 50x50=2500, g2d is 606us~870us, cpu is 777us~949us */
        return 2500;
    default:
        return 0;
    }
}

int sunxifb_g2d_blit_to_fb(uintptr_t src_buf, uint32_t src_w, uint32_t src_h,
        uint32_t src_cx, uint32_t src_cy, uint32_t src_cw, uint32_t src_ch,
        uintptr_t dst_buf, uint32_t dst_w, uint32_t dst_h, uint32_t dst_cx,
        uint32_t dst_cy, uint32_t dst_cw, uint32_t dst_ch,
        g2d_blt_flags_h rotated) {
    g2d_blt_h info;
    memset(&info, 0, sizeof(g2d_blt_h));

    info.flag_h = rotated;
    info.src_image_h.width = src_w;
    info.src_image_h.height = src_h;
    info.src_image_h.laddr[0] = src_buf;

    info.src_image_h.clip_rect.x = src_cx;
    info.src_image_h.clip_rect.y = src_cy;
    info.src_image_h.clip_rect.w = src_cw;
    info.src_image_h.clip_rect.h = src_ch;

    info.src_image_h.format = g_format;

    info.src_image_h.mode = G2D_GLOBAL_ALPHA;
    info.src_image_h.alpha = 255;
    info.src_image_h.color = 0xee8899;
    info.src_image_h.align[0] = 0;
    info.src_image_h.align[1] = info.src_image_h.align[0];
    info.src_image_h.align[2] = info.src_image_h.align[0];
    info.src_image_h.laddr[1] = (uintptr_t) 0;
    info.src_image_h.laddr[2] = (uintptr_t) 0;
    info.src_image_h.use_phy_addr = 1;

    info.dst_image_h.format = g_format;

    info.dst_image_h.width = dst_w;
    info.dst_image_h.height = dst_h;

    info.dst_image_h.clip_rect.x = dst_cx;
    info.dst_image_h.clip_rect.y = dst_cy;
    info.dst_image_h.clip_rect.w = dst_cw;
    info.dst_image_h.clip_rect.h = dst_ch;

    info.dst_image_h.mode = G2D_GLOBAL_ALPHA;
    info.dst_image_h.alpha = 255;
    info.dst_image_h.color = 0xee8899;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[0] = dst_buf;
    info.dst_image_h.laddr[1] = (uintptr_t) 0;
    info.dst_image_h.laddr[2] = (uintptr_t) 0;
    info.dst_image_h.use_phy_addr = 1;

#ifdef LV_USE_SUNXIFB_DEBUG
    printf(
            "sunxifb_g2d_blit_to_fb src[phy=%p format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                    "dst=[phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
            (void*) info.src_image_h.laddr[0], info.src_image_h.format,
            info.src_image_h.alpha, info.src_image_h.width,
            info.src_image_h.height, info.src_image_h.clip_rect.x,
            info.src_image_h.clip_rect.y, info.src_image_h.clip_rect.w,
            info.src_image_h.clip_rect.h, (void*) info.dst_image_h.laddr[0],
            info.dst_image_h.format, info.dst_image_h.width,
            info.dst_image_h.height, info.dst_image_h.clip_rect.x,
            info.dst_image_h.clip_rect.y, info.dst_image_h.clip_rect.w,
            info.dst_image_h.clip_rect.h);
#endif /* LV_USE_SUNXIFB_DEBUG */

    if (ioctl(g_g2dfd, G2D_CMD_BITBLT_H, (uintptr_t)(&info)) < 0) {
        perror("Error: sunxifb_g2d_blit_to_fb G2D_CMD_BITBLT_H failed");
        printf(
                "sunxifb_g2d_blit_to_fb src[phy=%p format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                        "dst=[phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
                (void*) info.src_image_h.laddr[0], info.src_image_h.format,
                info.src_image_h.alpha, info.src_image_h.width,
                info.src_image_h.height, info.src_image_h.clip_rect.x,
                info.src_image_h.clip_rect.y, info.src_image_h.clip_rect.w,
                info.src_image_h.clip_rect.h, (void*) info.dst_image_h.laddr[0],
                info.dst_image_h.format, info.dst_image_h.width,
                info.dst_image_h.height, info.dst_image_h.clip_rect.x,
                info.dst_image_h.clip_rect.y, info.dst_image_h.clip_rect.w,
                info.dst_image_h.clip_rect.h);
        return -1;
    }
    return 0;
}

#ifdef LV_USE_SUNXIFB_G2D_FILL
int sunxifb_g2d_fill(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t color, lv_opa_t opa) {
    g2d_fillrect_h info;
    memset(&info, 0, sizeof(g2d_fillrect_h));

    int32_t disp_w = lv_area_get_width(disp_area);
    int32_t disp_h = lv_area_get_height(disp_area);
    int32_t draw_area_w = lv_area_get_width(draw_area);
    int32_t draw_area_h = lv_area_get_height(draw_area);

    sunxifb_mem_flush_cache(dest_buf, disp_w * disp_h * sizeof(lv_color_t));

    if (opa > LV_OPA_MAX) {
        info.dst_image_h.alpha = 255;
    } else {
        info.dst_image_h.alpha = opa;
        color.ch.alpha = opa;
    }
    info.dst_image_h.mode = G2D_PIXEL_ALPHA;
    info.dst_image_h.color = color.full;
    info.dst_image_h.format = g_format;
    info.dst_image_h.clip_rect.x = draw_area->x1;
    info.dst_image_h.clip_rect.y = draw_area->y1;
    info.dst_image_h.clip_rect.w = draw_area_w;
    info.dst_image_h.clip_rect.h = draw_area_h;
    info.dst_image_h.width = disp_w;
    info.dst_image_h.height = disp_h;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(dest_buf);
    info.dst_image_h.laddr[1] = (uintptr_t) 0;
    info.dst_image_h.laddr[2] = (uintptr_t) 0;
    info.dst_image_h.use_phy_addr = 1;

#ifdef LV_USE_SUNXIFB_DEBUG
    printf(
            "sunxifb_g2d_fill dst=[vir=%p phy=%p color=%x alpha=%d format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
            dest_buf, (void*) info.dst_image_h.laddr[0], info.dst_image_h.color,
            info.dst_image_h.alpha, info.dst_image_h.format,
            info.dst_image_h.width, info.dst_image_h.height,
            info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
            info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);
#endif /* LV_USE_SUNXIFB_DEBUG */

    if (ioctl(g_g2dfd, G2D_CMD_FILLRECT_H, (uintptr_t)(&info)) < 0) {
        perror("ERROR: sunxifb_g2d_fill G2D_CMD_FILLRECT_H failed");
        printf(
                "sunxifb_g2d_fill dst=[vir=%p phy=%p color=%x alpha=%d format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
                dest_buf, (void*) info.dst_image_h.laddr[0],
                info.dst_image_h.color, info.dst_image_h.alpha,
                info.dst_image_h.format, info.dst_image_h.width,
                info.dst_image_h.height, info.dst_image_h.clip_rect.x,
                info.dst_image_h.clip_rect.y, info.dst_image_h.clip_rect.w,
                info.dst_image_h.clip_rect.h);
        return -1;
    }

    return 0;
}
#endif /* LV_USE_SUNXIFB_G2D_FILL */

#ifdef LV_USE_SUNXIFB_G2D_BLIT
int sunxifb_g2d_blit(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t *map, const lv_area_t *map_area,
        lv_opa_t opa) {
    g2d_blt_h info;
    memset(&info, 0, sizeof(g2d_blt_h));

    int32_t disp_w = lv_area_get_width(disp_area);
    int32_t disp_h = lv_area_get_height(disp_area);
    int32_t draw_area_w = lv_area_get_width(draw_area);
    int32_t draw_area_h = lv_area_get_height(draw_area);
    int32_t map_w = lv_area_get_width(map_area);
    int32_t map_h = lv_area_get_height(map_area);

    /* After reading the picture, refresh the cache instead of refreshing every time you compose */
    //sunxifb_mem_flush_cache(map, map_w * map_h * sizeof(lv_color_t));
    sunxifb_mem_flush_cache(dest_buf, disp_w * disp_h * sizeof(lv_color_t));

    if (opa > LV_OPA_MAX) {
        info.src_image_h.mode = G2D_PIXEL_ALPHA;
        info.src_image_h.alpha = 255;
    } else {
        info.src_image_h.mode = G2D_MIXER_ALPHA;
        info.src_image_h.alpha = opa;
    }

    info.flag_h = G2D_ROT_0;
    info.src_image_h.format = g_format;
    info.src_image_h.clip_rect.x = draw_area->x1
            - (map_area->x1 - disp_area->x1);
    info.src_image_h.clip_rect.y = draw_area->y1
            - (map_area->y1 - disp_area->y1);
    info.src_image_h.clip_rect.w = draw_area_w;
    info.src_image_h.clip_rect.h = draw_area_h;
    info.src_image_h.width = map_w;
    info.src_image_h.height = map_h;
    info.src_image_h.color = 0xee8899;
    info.src_image_h.align[0] = 0;
    info.src_image_h.align[1] = info.src_image_h.align[0];
    info.src_image_h.align[2] = info.src_image_h.align[0];
    info.src_image_h.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(map);
    info.src_image_h.laddr[1] = (uintptr_t) 0;
    info.src_image_h.laddr[2] = (uintptr_t) 0;
    info.src_image_h.use_phy_addr = 1;

    info.dst_image_h.format = g_format;
    info.dst_image_h.clip_rect.x = draw_area->x1;
    info.dst_image_h.clip_rect.y = draw_area->y1;
    info.dst_image_h.clip_rect.w = draw_area_w;
    info.dst_image_h.clip_rect.h = draw_area_h;
    info.dst_image_h.width = disp_w;
    info.dst_image_h.height = disp_h;
    info.dst_image_h.mode = G2D_GLOBAL_ALPHA;
    info.dst_image_h.alpha = 255;
    info.dst_image_h.color = 0xee8899;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(dest_buf);
    info.dst_image_h.laddr[1] = (uintptr_t) 0;
    info.dst_image_h.laddr[2] = (uintptr_t) 0;
    info.dst_image_h.use_phy_addr = 1;

#ifdef LV_USE_SUNXIFB_DEBUG
    printf(
            "sunxifb_g2d_blit src[vir=%p phy=%p format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                    "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
            map, (void*) info.src_image_h.laddr[0], info.src_image_h.format,
            info.src_image_h.alpha, info.src_image_h.width,
            info.src_image_h.height, info.src_image_h.clip_rect.x,
            info.src_image_h.clip_rect.y, info.src_image_h.clip_rect.w,
            info.src_image_h.clip_rect.h, dest_buf,
            (void*) info.dst_image_h.laddr[0], info.dst_image_h.format,
            info.dst_image_h.width, info.dst_image_h.height,
            info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
            info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);
#endif /* LV_USE_SUNXIFB_DEBUG */

    if (ioctl(g_g2dfd, G2D_CMD_BITBLT_H, (uintptr_t)(&info)) < 0) {
        perror("Error: sunxifb_g2d_blit G2D_CMD_BITBLT_H failed");
        printf(
                "sunxifb_g2d_blit src[vir=%p phy=%p format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                        "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
                map, (void*) info.src_image_h.laddr[0], info.src_image_h.format,
                info.src_image_h.alpha, info.src_image_h.width,
                info.src_image_h.height, info.src_image_h.clip_rect.x,
                info.src_image_h.clip_rect.y, info.src_image_h.clip_rect.w,
                info.src_image_h.clip_rect.h, dest_buf,
                (void*) info.dst_image_h.laddr[0], info.dst_image_h.format,
                info.dst_image_h.width, info.dst_image_h.height,
                info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
                info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);
        return -1;
    }
    return 0;
}
#endif /* LV_USE_SUNXIFB_G2D_BLIT */

#ifdef LV_USE_SUNXIFB_G2D_BLEND
int sunxifb_g2d_blend(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t *map, const lv_area_t *map_area,
        lv_opa_t opa, bool chroma_key) {
    g2d_bld info;
    memset(&info, 0, sizeof(g2d_bld));

    int32_t disp_w = lv_area_get_width(disp_area);
    int32_t disp_h = lv_area_get_height(disp_area);
    int32_t draw_area_w = lv_area_get_width(draw_area);
    int32_t draw_area_h = lv_area_get_height(draw_area);
    int32_t map_w = lv_area_get_width(map_area);
    int32_t map_h = lv_area_get_height(map_area);

    /* After reading the picture, refresh the cache instead of refreshing every time you compose */
    //sunxifb_mem_flush_cache(map, map_w * map_h * sizeof(lv_color_t));
    sunxifb_mem_flush_cache(dest_buf, disp_w * disp_h * sizeof(lv_color_t));

    if (chroma_key) {
        info.bld_cmd = G2D_CK_DST;
        info.ck_para.match_rule = 0;
        info.ck_para.max_color = color_key;
        info.ck_para.min_color = color_key;
    } else {
        info.bld_cmd = G2D_BLD_SRCOVER;
    }

#ifdef CONF_G2D_VERSION_NEW
    if (opa > LV_OPA_MAX) {
        info.src_image[1].mode = G2D_PIXEL_ALPHA;
        info.src_image[1].alpha = 255;
    } else {
        info.src_image[1].mode = G2D_MIXER_ALPHA;
        info.src_image[1].alpha = opa;
    }

    /* Calculate the clipping range, refer to the lv_draw_map function */
    info.src_image[1].format = g_format;
    info.src_image[1].clip_rect.x = draw_area->x1
            - (map_area->x1 - disp_area->x1);
    info.src_image[1].clip_rect.y = draw_area->y1
            - (map_area->y1 - disp_area->y1);
    info.src_image[1].clip_rect.w = draw_area_w;
    info.src_image[1].clip_rect.h = draw_area_h;
    info.src_image[1].width = map_w;
    info.src_image[1].height = map_h;
    info.src_image[1].color = 0xee8899;
    info.src_image[1].align[0] = 0;
    info.src_image[1].align[1] = info.src_image[0].align[0];
    info.src_image[1].align[2] = info.src_image[0].align[0];
    info.src_image[1].laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(map);
    info.src_image[1].laddr[1] = (uintptr_t) 0;
    info.src_image[1].laddr[2] = (uintptr_t) 0;
    info.src_image[1].use_phy_addr = 1;

    info.dst_image.format = g_format;
    info.dst_image.clip_rect.x = draw_area->x1;
    info.dst_image.clip_rect.y = draw_area->y1;
    info.dst_image.clip_rect.w = draw_area_w;
    info.dst_image.clip_rect.h = draw_area_h;
    info.dst_image.width = disp_w;
    info.dst_image.height = disp_h;
    info.dst_image.mode = G2D_PIXEL_ALPHA;
    info.dst_image.alpha = 255;
    info.dst_image.color = 0xee8899;
    info.dst_image.align[0] = 0;
    info.dst_image.align[1] = info.dst_image.align[0];
    info.dst_image.align[2] = info.dst_image.align[0];
    info.dst_image.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(dest_buf);
    info.dst_image.laddr[1] = (uintptr_t) 0;
    info.dst_image.laddr[2] = (uintptr_t) 0;
    info.dst_image.use_phy_addr = 1;

    /* src_image[1] is the top, src_image[0] is the bottom */
    /* src_image[0] is used as dst_image, no need to malloc a buffer */
    info.src_image[0] = info.dst_image;

#ifdef LV_USE_SUNXIFB_DEBUG
    printf(
            "sunxifb_g2d_blend "
                    "src=[vir=%p phy=%p cmd=%x format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                    "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
            map, (void*) info.src_image[1].laddr[0], info.bld_cmd,
            info.src_image[1].format, info.src_image[1].alpha,
            info.src_image[1].width, info.src_image[1].height,
            info.src_image[1].clip_rect.x, info.src_image[1].clip_rect.y,
            info.src_image[1].clip_rect.w, info.src_image[1].clip_rect.h,
            dest_buf, (void*) info.dst_image.laddr[0], info.dst_image.format,
            info.dst_image.width, info.dst_image.height,
            info.dst_image.clip_rect.x, info.dst_image.clip_rect.y,
            info.dst_image.clip_rect.w, info.dst_image.clip_rect.h);
#endif /* LV_USE_SUNXIFB_DEBUG */

    if (ioctl(g_g2dfd, G2D_CMD_BLD_H, (uintptr_t)(&info)) < 0) {
        perror("ERROR: sunxifb_g2d_blend G2D_CMD_BLD_H failed");
        printf(
                "sunxifb_g2d_blend "
                        "src=[vir=%p phy=%p cmd=%x format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                        "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
                map, (void*) info.src_image[1].laddr[0], info.bld_cmd,
                info.src_image[1].format, info.src_image[1].alpha,
                info.src_image[1].width, info.src_image[1].height,
                info.src_image[1].clip_rect.x, info.src_image[1].clip_rect.y,
                info.src_image[1].clip_rect.w, info.src_image[1].clip_rect.h,
                dest_buf, (void*) info.dst_image.laddr[0],
                info.dst_image.format, info.dst_image.width,
                info.dst_image.height, info.dst_image.clip_rect.x,
                info.dst_image.clip_rect.y, info.dst_image.clip_rect.w,
                info.dst_image.clip_rect.h);
        return -1;
    }
#else
    if (opa > LV_OPA_MAX) {
        info.src_image_h.mode = G2D_PIXEL_ALPHA;
        info.src_image_h.alpha = 255;
    } else {
        info.src_image_h.mode = G2D_MIXER_ALPHA;
        info.src_image_h.alpha = opa;
    }

    /* Calculate the clipping range, refer to the lv_draw_map function */
    info.src_image_h.format = g_format;
    info.src_image_h.clip_rect.x = draw_area->x1
            - (map_area->x1 - disp_area->x1);
    info.src_image_h.clip_rect.y = draw_area->y1
            - (map_area->y1 - disp_area->y1);
    info.src_image_h.clip_rect.w = draw_area_w;
    info.src_image_h.clip_rect.h = draw_area_h;
    info.src_image_h.width = map_w;
    info.src_image_h.height = map_h;
    info.src_image_h.color = 0xee8899;
    info.src_image_h.align[0] = 0;
    info.src_image_h.align[1] = info.src_image_h.align[0];
    info.src_image_h.align[2] = info.src_image_h.align[0];
    info.src_image_h.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(map);
    info.src_image_h.laddr[1] = (uintptr_t) 0;
    info.src_image_h.laddr[2] = (uintptr_t) 0;
    info.src_image_h.use_phy_addr = 1;

    info.dst_image_h.format = g_format;
    info.dst_image_h.clip_rect.x = draw_area->x1;
    info.dst_image_h.clip_rect.y = draw_area->y1;
    info.dst_image_h.clip_rect.w = draw_area_w;
    info.dst_image_h.clip_rect.h = draw_area_h;
    info.dst_image_h.width = disp_w;
    info.dst_image_h.height = disp_h;
    info.dst_image_h.mode = G2D_PIXEL_ALPHA;
    info.dst_image_h.alpha = 255;
    info.dst_image_h.color = 0xee8899;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(dest_buf);
    info.dst_image_h.laddr[1] = (uintptr_t) 0;
    info.dst_image_h.laddr[2] = (uintptr_t) 0;
    info.dst_image_h.use_phy_addr = 1;

#ifdef LV_USE_SUNXIFB_DEBUG
    printf(
            "sunxifb_g2d_blend "
                    "src=[vir=%p phy=%p cmd=%x format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                    "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
            map, (void*) info.src_image_h.laddr[0], info.bld_cmd,
            info.src_image_h.format, info.src_image_h.alpha,
            info.src_image_h.width, info.src_image_h.height,
            info.src_image_h.clip_rect.x, info.src_image_h.clip_rect.y,
            info.src_image_h.clip_rect.w, info.src_image_h.clip_rect.h,
            dest_buf, (void*) info.dst_image_h.laddr[0],
            info.dst_image_h.format, info.dst_image_h.width,
            info.dst_image_h.height, info.dst_image_h.clip_rect.x,
            info.dst_image_h.clip_rect.y, info.dst_image_h.clip_rect.w,
            info.dst_image_h.clip_rect.h);
#endif /* LV_USE_SUNXIFB_DEBUG */

    if (ioctl(g_g2dfd, G2D_CMD_BLD_H, (uintptr_t)(&info)) < 0) {
        perror("ERROR: sunxifb_g2d_blend G2D_CMD_BLD_H failed");
        printf(
                "sunxifb_g2d_blend "
                        "src=[vir=%p phy=%p cmd=%x format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                        "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
                map, (void*) info.src_image_h.laddr[0], info.bld_cmd,
                info.src_image_h.format, info.src_image_h.alpha,
                info.src_image_h.width, info.src_image_h.height,
                info.src_image_h.clip_rect.x, info.src_image_h.clip_rect.y,
                info.src_image_h.clip_rect.w, info.src_image_h.clip_rect.h,
                dest_buf, (void*) info.dst_image_h.laddr[0],
                info.dst_image_h.format, info.dst_image_h.width,
                info.dst_image_h.height, info.dst_image_h.clip_rect.x,
                info.dst_image_h.clip_rect.y, info.dst_image_h.clip_rect.w,
                info.dst_image_h.clip_rect.h);
        return -1;
    }
#endif

    return 0;
}
#endif /* LV_USE_SUNXIFB_G2D_BLEND */

#ifdef LV_USE_SUNXIFB_G2D_SCALE
int sunxifb_g2d_scale(lv_color_t *dest_buf, const lv_area_t *disp_area,
        const lv_area_t *draw_area, lv_color_t *map, const lv_area_t *map_area,
        lv_opa_t opa, uint16_t zoom, const lv_point_t *pivot) {
    g2d_blt_h info;
    lv_area_t zoom_area;

    memset(&info, 0, sizeof(g2d_blt_h));

    int32_t map_w = lv_area_get_width(map_area);
    int32_t map_h = lv_area_get_height(map_area);

    /* Calculate the zoom size and clipping range, refer to the lv_img_draw_core function */
    _lv_img_buf_get_transformed_area(&zoom_area, map_w, map_h, 0, zoom, pivot);
    zoom_area.x1 += map_area->x1;
    zoom_area.y1 += map_area->y1;
    zoom_area.x2 += map_area->x1;
    zoom_area.y2 += map_area->y1;

    int32_t zoom_w = lv_area_get_width(&zoom_area);
    int32_t zoom_h = lv_area_get_height(&zoom_area);

    lv_color_t *scale_buf = (lv_color_t*) sunxifb_mem_alloc(
            zoom_w * zoom_h * sizeof(lv_color_t), "sunxifb_g2d_scale");

    if (NULL == scale_buf)
        return -1;

    if (opa > LV_OPA_MAX) {
        info.src_image_h.mode = G2D_PIXEL_ALPHA;
        info.src_image_h.alpha = 255;
    } else {
        info.src_image_h.mode = G2D_MIXER_ALPHA;
        info.src_image_h.alpha = opa;
    }

    info.flag_h = G2D_BLT_NONE_H;
    info.src_image_h.format = g_format;
    info.src_image_h.clip_rect.x = 0;
    info.src_image_h.clip_rect.y = 0;
    info.src_image_h.clip_rect.w = map_w;
    info.src_image_h.clip_rect.h = map_h;
    info.src_image_h.width = map_w;
    info.src_image_h.height = map_h;
    info.src_image_h.color = 0xee8899;
    info.src_image_h.align[0] = 0;
    info.src_image_h.align[1] = info.src_image_h.align[0];
    info.src_image_h.align[2] = info.src_image_h.align[0];
    info.src_image_h.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(map);
    info.src_image_h.laddr[1] = (uintptr_t) 0;
    info.src_image_h.laddr[2] = (uintptr_t) 0;
    info.src_image_h.use_phy_addr = 1;

    info.dst_image_h.format = g_format;
    info.dst_image_h.clip_rect.x = 0;
    info.dst_image_h.clip_rect.y = 0;
    info.dst_image_h.clip_rect.w = zoom_w;
    info.dst_image_h.clip_rect.h = zoom_h;
    info.dst_image_h.width = zoom_w;
    info.dst_image_h.height = zoom_h;
    info.dst_image_h.mode = G2D_GLOBAL_ALPHA;
    info.dst_image_h.alpha = 255;
    info.dst_image_h.color = 0xee8899;
    info.dst_image_h.align[0] = 0;
    info.dst_image_h.align[1] = info.dst_image_h.align[0];
    info.dst_image_h.align[2] = info.dst_image_h.align[0];
    info.dst_image_h.laddr[0] = (uintptr_t) sunxifb_mem_get_phyaddr(scale_buf);
    info.dst_image_h.laddr[1] = (uintptr_t) 0;
    info.dst_image_h.laddr[2] = (uintptr_t) 0;
    info.dst_image_h.use_phy_addr = 1;

#ifdef LV_USE_SUNXIFB_DEBUG
    printf(
            "sunxifb_g2d_scale src[vir=%p phy=%p format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                    "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
            map, (void*) info.src_image_h.laddr[0], info.src_image_h.format,
            info.src_image_h.alpha, info.src_image_h.width,
            info.src_image_h.height, info.src_image_h.clip_rect.x,
            info.src_image_h.clip_rect.y, info.src_image_h.clip_rect.w,
            info.src_image_h.clip_rect.h, dest_buf,
            (void*) info.dst_image_h.laddr[0], info.dst_image_h.format,
            info.dst_image_h.width, info.dst_image_h.height,
            info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
            info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);
#endif /* LV_USE_SUNXIFB_DEBUG */

    if (ioctl(g_g2dfd, G2D_CMD_BITBLT_H, (uintptr_t)(&info)) < 0) {
        perror("Error: sunxifb_g2d_scale G2D_CMD_BITBLT_H failed");
        printf(
                "sunxifb_g2d_scale src[vir=%p phy=%p format=%d alpha=%d wh=[%d %d] clip=[%d %d %d %d]] "
                        "dst=[vir=%p phy=%p format=%d wh=[%d %d] clip=[%d %d %d %d]]\n",
                map, (void*) info.src_image_h.laddr[0], info.src_image_h.format,
                info.src_image_h.alpha, info.src_image_h.width,
                info.src_image_h.height, info.src_image_h.clip_rect.x,
                info.src_image_h.clip_rect.y, info.src_image_h.clip_rect.w,
                info.src_image_h.clip_rect.h, dest_buf,
                (void*) info.dst_image_h.laddr[0], info.dst_image_h.format,
                info.dst_image_h.width, info.dst_image_h.height,
                info.dst_image_h.clip_rect.x, info.dst_image_h.clip_rect.y,
                info.dst_image_h.clip_rect.w, info.dst_image_h.clip_rect.h);
        sunxifb_mem_free((void**) &scale_buf, "sunxifb_g2d_scale");
        return -1;
    }

#ifdef LV_USE_SUNXIFB_G2D_BLEND
    sunxifb_g2d_blend(dest_buf, disp_area, draw_area, scale_buf, &zoom_area,
            opa, false);
#endif

    sunxifb_mem_free((void**) &scale_buf, "sunxifb_g2d_scale");
    return 0;
}
#endif /* LV_USE_SUNXIFB_G2D_SCALE */

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif
