#include "recorder.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <climits>
#include <cmath>

#include "utils/tiny_jpeg.h" 

#include "my_main.h" 



#include "src/extra/others/snapshot/lv_snapshot.h"





static inline uint8_t clamp_u8(int v) { return (v < 0) ? 0 : (v > 255 ? 255 : v); }

static inline void blend_pixel_bgra(uint8_t *p, uint8_t b, uint8_t g, uint8_t r, uint8_t a)
{
    
    
    if (a == 255)
    {
        p[0] = b;
        p[1] = g;
        p[2] = r;
        p[3] = 255;
        return;
    }
    if (a == 0) return;

    const uint32_t inv = 255 - a;
    p[0] = (uint8_t)((b * a + p[0] * inv) / 255);
    p[1] = (uint8_t)((g * a + p[1] * inv) / 255);
    p[2] = (uint8_t)((r * a + p[2] * inv) / 255);
    p[3] = 255;
}















extern "C" const lv_img_dsc_t crosshairs;

typedef struct
{
    int inited;

    
    lv_obj_t *scr;

    
    lv_obj_t *max_obj;
    lv_obj_t *min_obj;
    lv_obj_t *img_max;
    lv_obj_t *img_min;
    lv_obj_t *lbl_max;
    lv_obj_t *lbl_min;

    
    lv_obj_t *graph_card;
    lv_obj_t *graph_chart;
    lv_chart_series_t *graph_series_min;
    lv_chart_series_t *graph_series_max;

    int graph_w;
    int graph_h;
    uint64_t graph_last_push_ms;

    
    void *snap_buf;
    uint32_t snap_buf_size;
    lv_img_dsc_t snap_dsc;
} lvgl_overlay_t;

static lvgl_overlay_t g_lvgl_overlay = {};



static uint64_t now_ms();

static void lvgl_overlay_init_locked(int width, int height)
{
    if (g_lvgl_overlay.inited)
        return;

    
    if (width != 320 || height != 240)
        return;

    g_lvgl_overlay.scr = lv_obj_create(NULL);
    lv_obj_set_size(g_lvgl_overlay.scr, width, height);
    lv_obj_clear_flag(g_lvgl_overlay.scr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(g_lvgl_overlay.scr, 0, 0);
    lv_obj_set_style_border_width(g_lvgl_overlay.scr, 0, 0);
    lv_obj_set_style_radius(g_lvgl_overlay.scr, 0, 0);
    lv_obj_set_style_bg_opa(g_lvgl_overlay.scr, LV_OPA_TRANSP, 0);

    
    auto make_crosshair = [&](lv_obj_t **out_obj, lv_obj_t **out_img, lv_obj_t **out_lbl) {
        *out_obj = lv_obj_create(g_lvgl_overlay.scr);
        lv_obj_set_size(*out_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(*out_obj, 0, 0);
        lv_obj_set_style_bg_opa(*out_obj, 0, 0);
        lv_obj_set_style_border_width(*out_obj, 0, 0);
        lv_obj_set_style_radius(*out_obj, 0, 0);
        lv_obj_add_flag(*out_obj, LV_OBJ_FLAG_HIDDEN);

        *out_img = lv_img_create(*out_obj);
        lv_img_set_src(*out_img, &crosshairs);

        *out_lbl = lv_label_create(*out_obj);
        lv_obj_set_style_bg_color(*out_lbl, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(*out_lbl, LV_OPA_50, 0);
        lv_obj_set_style_radius(*out_lbl, 3, 0);
        lv_obj_align_to(*out_lbl, *out_img, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
    };

    make_crosshair(&g_lvgl_overlay.max_obj, &g_lvgl_overlay.img_max, &g_lvgl_overlay.lbl_max);
    make_crosshair(&g_lvgl_overlay.min_obj, &g_lvgl_overlay.img_min, &g_lvgl_overlay.lbl_min);

    
    g_lvgl_overlay.graph_h = 120;
    g_lvgl_overlay.graph_w = 0; 
    g_lvgl_overlay.graph_last_push_ms = 0;

    g_lvgl_overlay.graph_card = lv_obj_create(g_lvgl_overlay.scr);
    lv_obj_set_style_pad_all(g_lvgl_overlay.graph_card, 0, 0);
    lv_obj_set_style_bg_opa(g_lvgl_overlay.graph_card, 192, 0);
    lv_obj_clear_flag(g_lvgl_overlay.graph_card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(g_lvgl_overlay.graph_card, true, 0);

    g_lvgl_overlay.graph_chart = lv_chart_create(g_lvgl_overlay.graph_card);
    lv_obj_set_style_bg_opa(g_lvgl_overlay.graph_chart, 0, 0);
    lv_obj_set_style_border_width(g_lvgl_overlay.graph_chart, 0, 0);
    lv_obj_set_style_pad_all(g_lvgl_overlay.graph_chart, 0, 0);
    lv_obj_clear_flag(g_lvgl_overlay.graph_chart, LV_OBJ_FLAG_SCROLLABLE);

    lv_chart_set_update_mode(g_lvgl_overlay.graph_chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_type(g_lvgl_overlay.graph_chart, LV_CHART_TYPE_LINE);
    lv_obj_set_style_line_width(g_lvgl_overlay.graph_chart, 1, LV_PART_ITEMS);
    lv_chart_set_range(g_lvgl_overlay.graph_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    lv_chart_set_range(g_lvgl_overlay.graph_chart, LV_CHART_AXIS_SECONDARY_Y, 0, 1000);
    lv_obj_set_style_opa(g_lvgl_overlay.graph_chart, LV_OPA_0, LV_PART_INDICATOR);

    g_lvgl_overlay.graph_series_min = lv_chart_add_series(g_lvgl_overlay.graph_chart,
                                                         lv_palette_main(LV_PALETTE_BLUE),
                                                         LV_CHART_AXIS_PRIMARY_Y);
    g_lvgl_overlay.graph_series_max = lv_chart_add_series(g_lvgl_overlay.graph_chart,
                                                         lv_palette_main(LV_PALETTE_RED),
                                                         LV_CHART_AXIS_SECONDARY_Y);

    
    g_lvgl_overlay.snap_buf = NULL;
    g_lvgl_overlay.snap_buf_size = 0;
    lv_memset_00(&g_lvgl_overlay.snap_dsc, sizeof(g_lvgl_overlay.snap_dsc));

    g_lvgl_overlay.inited = 1;
}

static void lvgl_overlay_config_graph_locked(int img_w, int img_h)
{
    const int want_w = globalSettings.graphSize ? 240 : 120;
    const int want_h = 120;

    if (g_lvgl_overlay.graph_w != want_w)
    {
        g_lvgl_overlay.graph_w = want_w;
        g_lvgl_overlay.graph_h = want_h;

        lv_obj_set_size(g_lvgl_overlay.graph_card, want_w, want_h);
        lv_obj_set_size(g_lvgl_overlay.graph_chart, want_w, want_h);

        lv_chart_set_point_count(g_lvgl_overlay.graph_chart, want_w);
        lv_chart_set_all_value(g_lvgl_overlay.graph_chart, g_lvgl_overlay.graph_series_min, 0);
        lv_chart_set_all_value(g_lvgl_overlay.graph_chart, g_lvgl_overlay.graph_series_max, 0);

        g_lvgl_overlay.graph_last_push_ms = 0;
    }

    
    int x = 10;
    int y = 10;
    int pos = globalSettings.graphPos;
    if (pos == 1 || pos == 2) x = img_w - want_w - 10;
    if (pos == 2 || pos == 3) y = img_h - want_h - 10;
    lv_obj_set_pos(g_lvgl_overlay.graph_card, x, y);
}

static void lvgl_overlay_update_locked(int img_w, int img_h)
{
    
    const bool view_allows_crosshairs = (current_mode == MODE_MAINPAGE || current_mode == MODE_CAMERA_SETTINGS);

    
    if (globalSettings.use4117Cursors || !view_allows_crosshairs || !globalSettings.enableMaxValueDisplay)
    {
        lv_obj_add_flag(g_lvgl_overlay.max_obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(g_lvgl_overlay.max_obj, LV_OBJ_FLAG_HIDDEN);

        float x = cameraUtils.lastResult.MaxTemperaturePoint.positionX;
        float y = cameraUtils.lastResult.MaxTemperaturePoint.positionY;
        if (!std::isfinite(x)) x = 0.5f;
        if (!std::isfinite(y)) y = 0.5f;
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        if (y < 0.0f) y = 0.0f;
        if (y > 1.0f) y = 1.0f;

        int px = (int)lroundf(x * 320.0f) - 7;
        int py = (int)lroundf(y * 240.0f) - 7;
        lv_obj_set_pos(g_lvgl_overlay.max_obj, px, py);

        char buf[16];
        sprintf(buf, "%.1f", cameraUtils.lastResult.maxTemperature);
        lv_label_set_text(g_lvgl_overlay.lbl_max, buf);
    }

    
    if (globalSettings.use4117Cursors || !view_allows_crosshairs || !globalSettings.enableMinValueDisplay)
    {
        lv_obj_add_flag(g_lvgl_overlay.min_obj, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(g_lvgl_overlay.min_obj, LV_OBJ_FLAG_HIDDEN);

        float x = cameraUtils.lastResult.MinTemperaturePoint.positionX;
        float y = cameraUtils.lastResult.MinTemperaturePoint.positionY;
        if (!std::isfinite(x)) x = 0.5f;
        if (!std::isfinite(y)) y = 0.5f;
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        if (y < 0.0f) y = 0.0f;
        if (y > 1.0f) y = 1.0f;

        int px = (int)lroundf(x * 320.0f) - 7;
        int py = (int)lroundf(y * 240.0f) - 7;
        lv_obj_set_pos(g_lvgl_overlay.min_obj, px, py);

        char buf[16];
        sprintf(buf, "%.1f", cameraUtils.lastResult.minTemperature);
        lv_label_set_text(g_lvgl_overlay.lbl_min, buf);
    }

    
    const bool graph_visible = (globalSettings.enableGraph != 0) && (current_mode != MODE_GALLERY);
    if (!graph_visible)
    {
        lv_obj_add_flag(g_lvgl_overlay.graph_card, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(g_lvgl_overlay.graph_card, LV_OBJ_FLAG_HIDDEN);
    lvgl_overlay_config_graph_locked(img_w, img_h);

    
    static const int interval_table[7] = {40, 100, 500, 1000, 2000, 5000, 10000};
    const int interval = interval_table[globalSettings.graphRefreshInterval % 7];
    const uint64_t t = now_ms();
    if (g_lvgl_overlay.graph_last_push_ms == 0)
        g_lvgl_overlay.graph_last_push_ms = t;

    if (t - g_lvgl_overlay.graph_last_push_ms >= (uint64_t)interval)
    {
        const float fmin = std::isfinite(cameraUtils.lastResult.minTemperature) ? cameraUtils.lastResult.minTemperature : 0.0f;
        const float fmax = std::isfinite(cameraUtils.lastResult.maxTemperature) ? cameraUtils.lastResult.maxTemperature : 0.0f;
        const int32_t vmin = (int32_t)lroundf(fmin * 100.0f);
        const int32_t vmax = (int32_t)lroundf(fmax * 100.0f);

        lv_chart_set_next_value(g_lvgl_overlay.graph_chart, g_lvgl_overlay.graph_series_min, (lv_coord_t)vmin);
        lv_chart_set_next_value(g_lvgl_overlay.graph_chart, g_lvgl_overlay.graph_series_max, (lv_coord_t)vmax);

        
        lv_coord_t *ser1 = lv_chart_get_y_array(g_lvgl_overlay.graph_chart, g_lvgl_overlay.graph_series_min);
        lv_coord_t *ser2 = lv_chart_get_y_array(g_lvgl_overlay.graph_chart, g_lvgl_overlay.graph_series_max);
        const int sz = (int)lv_chart_get_point_count(g_lvgl_overlay.graph_chart);
        lv_coord_t max_num = INT16_MIN, min_num = INT16_MAX;
        for (int i = 0; i < sz; ++i)
        {
            if (ser1[i] > max_num) max_num = ser1[i];
            if (ser1[i] < min_num) min_num = ser1[i];
            if (ser2[i] > max_num) max_num = ser2[i];
            if (ser2[i] < min_num) min_num = ser2[i];
        }
        lv_chart_set_range(g_lvgl_overlay.graph_chart, LV_CHART_AXIS_PRIMARY_Y, min_num - 50, max_num + 50);
        lv_chart_set_range(g_lvgl_overlay.graph_chart, LV_CHART_AXIS_SECONDARY_Y, min_num - 50, max_num + 50);

        g_lvgl_overlay.graph_last_push_ms = t;
    }
}

static void blend_overlay_img_bgra(uint8_t *dst_bgra, int dst_stride, int dst_w, int dst_h,
                                  const lv_img_dsc_t *overlay)
{
    if (!dst_bgra || !overlay || !overlay->data)
        return;

    const int ow = (int)overlay->header.w;
    const int oh = (int)overlay->header.h;
    const int bw = (ow < dst_w) ? ow : dst_w;
    const int bh = (oh < dst_h) ? oh : dst_h;
    if (bw <= 0 || bh <= 0)
        return;

    const uint8_t *src = (const uint8_t *)overlay->data;
    const int src_stride = ow * 4;

    for (int y = 0; y < bh; ++y)
    {
        uint8_t *d = dst_bgra + y * dst_stride;
        const uint8_t *s = src + y * src_stride;
        for (int x = 0; x < bw; ++x)
        {
            const uint8_t b = s[0];
            const uint8_t g = s[1];
            const uint8_t r = s[2];
            const uint8_t a = s[3];
            if (a)
                blend_pixel_bgra(d, b, g, r, a);
            s += 4;
            d += 4;
        }
    }
}

static bool overlay_lvgl_burnin_bgra(uint8_t *dst_bgra, int dst_stride, int width, int height)
{
    if (!dst_bgra || dst_stride <= 0)
        return false;
    if (width != 320 || height != 240)
        return false;

    
    LOCKLV();
    lvgl_overlay_init_locked(width, height);
    if (!g_lvgl_overlay.inited)
    {
        UNLOCKLV();
        return false;
    }

    lvgl_overlay_update_locked(width, height);

    const uint32_t need = lv_snapshot_buf_size_needed(g_lvgl_overlay.scr, LV_IMG_CF_TRUE_COLOR_ALPHA);
    if (need == 0)
    {
        UNLOCKLV();
        return false;
    }
    if (!g_lvgl_overlay.snap_buf || g_lvgl_overlay.snap_buf_size < need)
    {
        void *p = realloc(g_lvgl_overlay.snap_buf, need);
        if (!p)
        {
            UNLOCKLV();
            return false;
        }
        g_lvgl_overlay.snap_buf = p;
        g_lvgl_overlay.snap_buf_size = need;
    }

    const lv_res_t r = lv_snapshot_take_to_buf(g_lvgl_overlay.scr, LV_IMG_CF_TRUE_COLOR_ALPHA,
                                              &g_lvgl_overlay.snap_dsc,
                                              g_lvgl_overlay.snap_buf, g_lvgl_overlay.snap_buf_size);
    UNLOCKLV();

    if (r != LV_RES_OK)
        return false;

    
    blend_overlay_img_bgra(dst_bgra, dst_stride, width, height, &g_lvgl_overlay.snap_dsc);
    return true;
}

static void fill_rect_bgra(uint8_t *img, int stride, int w, int h, int x, int y, int rw, int rh,
                           uint8_t b, uint8_t g, uint8_t r, uint8_t a)
{
    if (rw <= 0 || rh <= 0) return;
    if (x < 0) { rw += x; x = 0; }
    if (y < 0) { rh += y; y = 0; }
    if (x + rw > w) rw = w - x;
    if (y + rh > h) rh = h - y;
    if (rw <= 0 || rh <= 0) return;

    for (int yy = 0; yy < rh; ++yy)
    {
        uint8_t *row = img + (y + yy) * stride + x * 4;
        for (int xx = 0; xx < rw; ++xx)
        {
            blend_pixel_bgra(row + xx * 4, b, g, r, a);
        }
    }
}

static void fill_round_rect_bgra(uint8_t *img, int stride, int w, int h,
                                 int x, int y, int rw, int rh, int rad,
                                 uint8_t b, uint8_t g, uint8_t r, uint8_t a)
{
    if (!img || stride <= 0 || w <= 0 || h <= 0) return;
    if (rw <= 0 || rh <= 0) return;
    if (rad <= 0) { fill_rect_bgra(img, stride, w, h, x, y, rw, rh, b, g, r, a); return; }

    
    const int max_rad = (rw < rh ? rw : rh) / 2;
    if (rad > max_rad) rad = max_rad;

    
    const int x0 = (x < 0) ? 0 : x;
    const int y0 = (y < 0) ? 0 : y;
    const int x1 = (x + rw > w) ? w : (x + rw);
    const int y1 = (y + rh > h) ? h : (y + rh);
    if (x0 >= x1 || y0 >= y1) return;

    const int rad2 = rad * rad;

    for (int yy = y0; yy < y1; ++yy)
    {
        for (int xx = x0; xx < x1; ++xx)
        {
            
            const int lx = xx - x;
            const int ty = yy - y;
            const int rx = (x + rw - 1) - xx;
            const int by = (y + rh - 1) - yy;

            bool inside = true;

            
            if (lx < rad && ty < rad)
            {
                const int dx = rad - 1 - lx;
                const int dy = rad - 1 - ty;
                inside = (dx * dx + dy * dy) <= rad2;
            }
            
            else if (rx < rad && ty < rad)
            {
                const int dx = rad - 1 - rx;
                const int dy = rad - 1 - ty;
                inside = (dx * dx + dy * dy) <= rad2;
            }
            
            else if (lx < rad && by < rad)
            {
                const int dx = rad - 1 - lx;
                const int dy = rad - 1 - by;
                inside = (dx * dx + dy * dy) <= rad2;
            }
            
            else if (rx < rad && by < rad)
            {
                const int dx = rad - 1 - rx;
                const int dy = rad - 1 - by;
                inside = (dx * dx + dy * dy) <= rad2;
            }

            if (inside)
            {
                blend_pixel_bgra(img + yy * stride + xx * 4, b, g, r, a);
            }
        }
    }
}

static void draw_line_bgra(uint8_t *img, int stride, int w, int h,
                           int x0, int y0, int x1, int y1,
                           uint8_t b, uint8_t g, uint8_t r, uint8_t a)
{
    
    int64_t x0l = (int64_t)x0, y0l = (int64_t)y0;
    int64_t x1l = (int64_t)x1, y1l = (int64_t)y1;

    int64_t dx = llabs(x1l - x0l);
    int64_t sx = (x0l < x1l) ? 1 : -1;
    int64_t dy = -llabs(y1l - y0l);
    int64_t sy = (y0l < y1l) ? 1 : -1;

    
    int64_t max_steps = (dx > -dy) ? dx : -dy;
    if (max_steps > (int64_t)(w + h) * 16) return;

    int64_t err = dx + dy;

    while (1)
    {
        if ((uint64_t)x0l < (uint64_t)w && (uint64_t)y0l < (uint64_t)h)
        {
            blend_pixel_bgra(img + (int)y0l * stride + (int)x0l * 4, b, g, r, a);
        }
        if (x0l == x1l && y0l == y1l) break;
        int64_t e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0l += sx; }
        if (e2 <= dx) { err += dx; y0l += sy; }
    }
}


static const uint8_t font6x8[12][8] = {
    
    {0x3E,0x63,0x73,0x7B,0x6F,0x67,0x3E,0x00},
    
    {0x0C,0x1C,0x0C,0x0C,0x0C,0x0C,0x3F,0x00},
    
    {0x3E,0x63,0x03,0x0E,0x38,0x60,0x7F,0x00},
    
    {0x3E,0x63,0x03,0x1E,0x03,0x63,0x3E,0x00},
    
    {0x06,0x0E,0x1E,0x36,0x66,0x7F,0x06,0x00},
    
    {0x7F,0x60,0x7E,0x03,0x03,0x63,0x3E,0x00},
    
    {0x1E,0x30,0x60,0x7E,0x63,0x63,0x3E,0x00},
    
    {0x7F,0x63,0x06,0x0C,0x18,0x18,0x18,0x00},
    
    {0x3E,0x63,0x63,0x3E,0x63,0x63,0x3E,0x00},
    
    {0x3E,0x63,0x63,0x3F,0x03,0x06,0x3C,0x00},
    
    {0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x00},
    
    {0x00,0x00,0x00,0x3F,0x00,0x00,0x00,0x00},
};

static int glyph_index(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c == '.') return 10;
    if (c == '-') return 11;
    return -1;
}

static void draw_text6x8_bgra(uint8_t *img, int stride, int w, int h,
                              int x, int y, const char *text,
                              uint8_t b, uint8_t g, uint8_t r, uint8_t a)
{
    int cx = x;
    for (const char *p = text; *p; ++p)
    {
        int gi = glyph_index(*p);
        if (gi < 0) { cx += 6; continue; }
        for (int yy = 0; yy < 8; ++yy)
        {
            int py = y + yy;
            if ((unsigned)py >= (unsigned)h) continue;
            uint8_t bits = font6x8[gi][yy];
            for (int xx = 0; xx < 6; ++xx)
            {
                int px = cx + xx;
                if ((unsigned)px >= (unsigned)w) continue;
                
                if (bits & (1 << (7 - xx)))
                {
                    blend_pixel_bgra(img + py * stride + px * 4, b, g, r, a);
                }
            }
        }
        cx += 6;
    }
}

static void draw_crosshair_bgra(uint8_t *img, int stride, int w, int h, int cx, int cy,
                                uint8_t b, uint8_t g, uint8_t r)
{
    
    const int s = 6;
    
    draw_line_bgra(img, stride, w, h, cx - s, cy, cx + s, cy, 0, 0, 0, 80);
    draw_line_bgra(img, stride, w, h, cx, cy - s, cx, cy + s, 0, 0, 0, 80);
    
    draw_line_bgra(img, stride, w, h, cx - s, cy, cx + s, cy, b, g, r, 255);
    draw_line_bgra(img, stride, w, h, cx, cy - s, cx, cy + s, b, g, r, 255);
    
    fill_rect_bgra(img, stride, w, h, cx, cy, 1, 1, b, g, r, 255);
}


typedef struct
{
    int w;
    int h;
    int enabled;
    int inited;
    int16_t *min_buf;
    int16_t *max_buf;
    int idx;
    uint64_t last_push_ms;
} graph_state_t;

static uint64_t now_ms()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)tv.tv_usec / 1000ULL;
}

static void graph_state_reset(graph_state_t *gs, int w)
{
    if (gs->min_buf) free(gs->min_buf);
    if (gs->max_buf) free(gs->max_buf);
    gs->min_buf = (int16_t *)calloc(w, sizeof(int16_t));
    gs->max_buf = (int16_t *)calloc(w, sizeof(int16_t));
    gs->w = w;
    gs->h = 120;
    gs->idx = 0;
    gs->last_push_ms = 0;
    gs->inited = (gs->min_buf && gs->max_buf) ? 1 : 0;
}

static void graph_push(graph_state_t *gs, int16_t tmin, int16_t tmax)
{
    if (!gs->inited) return;
    
    memmove(gs->min_buf, gs->min_buf + 1, (gs->w - 1) * sizeof(int16_t));
    memmove(gs->max_buf, gs->max_buf + 1, (gs->w - 1) * sizeof(int16_t));
    gs->min_buf[gs->w - 1] = tmin;
    gs->max_buf[gs->w - 1] = tmax;
}
static inline float clamp_f(float v, float lo, float hi);
static inline float finite_or(float v, float fallback);
static void overlay_graph_bgra(uint8_t *img, int stride, int img_w, int img_h, graph_state_t *gs)
{
    if (!globalSettings.enableGraph) return;

    const int w = globalSettings.graphSize ? 240 : 120;
    const int h = 120;
    if (!gs->inited || gs->w != w)
    {
        graph_state_reset(gs, w);
    }
    if (!gs->inited) return;

    
    int x = 10;
    int y = 10;
    int pos = globalSettings.graphPos;
    if (pos == 1 || pos == 2) x = img_w - w - 10;
    if (pos == 2 || pos == 3) y = img_h - h - 10;

    
    static const int interval_table[7] = {40, 100, 500, 1000, 2000, 5000, 10000};
    int interval = interval_table[globalSettings.graphRefreshInterval % 7];
    uint64_t t = now_ms();
    if (gs->last_push_ms == 0) gs->last_push_ms = t;

    if (t - gs->last_push_ms >= (uint64_t)interval)
    {
        
        float fmin = finite_or(cameraUtils.lastResult.minTemperature, 0.0f);
        float fmax = finite_or(cameraUtils.lastResult.maxTemperature, 0.0f);
        int32_t cmin = (int32_t)lroundf(fmin * 100.0f);
        int32_t cmax = (int32_t)lroundf(fmax * 100.0f);
        if (cmin < INT16_MIN) cmin = INT16_MIN;
        if (cmin > INT16_MAX) cmin = INT16_MAX;
        if (cmax < INT16_MIN) cmax = INT16_MIN;
        if (cmax > INT16_MAX) cmax = INT16_MAX;
        graph_push(gs, (int16_t)cmin, (int16_t)cmax);
        gs->last_push_ms = t;
    }

    
    fill_rect_bgra(img, stride, img_w, img_h, x, y, w, h, 30, 30, 30, 192);

    
    int16_t mn = INT16_MAX, mx = INT16_MIN;
    for (int i = 0; i < w; ++i)
    {
        if (gs->min_buf[i] < mn) mn = gs->min_buf[i];
        if (gs->max_buf[i] < mn) mn = gs->max_buf[i];
        if (gs->min_buf[i] > mx) mx = gs->min_buf[i];
        if (gs->max_buf[i] > mx) mx = gs->max_buf[i];
    }
    int range_min = (int)mn - 50;
    int range_max = (int)mx + 50;
    if (range_max <= range_min) { range_max = range_min + 1; }

    auto map_y = [&](int16_t v) -> int {
        int yy = (range_max - (int)v) * (h - 1) / (range_max - range_min);
        if (yy < 0) yy = 0;
        if (yy >= h) yy = h - 1;
        return yy;
    };

    
    for (int i = 1; i < w; ++i)
    {
        int x0 = x + i - 1;
        int x1 = x + i;
        int y0_min = y + map_y(gs->min_buf[i - 1]);
        int y1_min = y + map_y(gs->min_buf[i]);
        int y0_max = y + map_y(gs->max_buf[i - 1]);
        int y1_max = y + map_y(gs->max_buf[i]);
        
        draw_line_bgra(img, stride, img_w, img_h, x0, y0_min, x1, y1_min, 255, 64, 64, 255);
        
        draw_line_bgra(img, stride, img_w, img_h, x0, y0_max, x1, y1_max, 64, 64, 255, 255);
    }
}

static inline float clamp_f(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline float finite_or(float v, float fallback)
{
    return std::isfinite(v) ? v : fallback;
}

static void overlay_crosshairs_bgra(uint8_t *img, int stride, int w, int h)
{
    
    if (!img || w <= 0 || h <= 0 || stride <= 0) return;

    char buf[16];

    if (globalSettings.enableMaxValueDisplay)
    {
        float px = finite_or(cameraUtils.lastResult.MaxTemperaturePoint.positionX, 0.5f);
        float py = finite_or(cameraUtils.lastResult.MaxTemperaturePoint.positionY, 0.5f);
        px = clamp_f(px, 0.0f, 1.0f);
        py = clamp_f(py, 0.0f, 1.0f);

        int cx = (int)lroundf(px * (float)(w - 1));
        int cy = (int)lroundf(py * (float)(h - 1));
        draw_crosshair_bgra(img, stride, w, h, cx, cy, 0, 255, 0);

        float t = finite_or(cameraUtils.lastResult.maxTemperature, 0.0f);
        snprintf(buf, sizeof(buf), "%.1f", t);
        int tx = cx + 8;
        int ty = cy - 4;
        int tw = (int)strlen(buf) * 6;
        
        fill_round_rect_bgra(img, stride, w, h, tx - 2, ty - 2, tw + 4, 12, 3, 0, 0, 0, 140);
        
        draw_text6x8_bgra(img, stride, w, h, tx + 1, ty + 1, buf, 0, 0, 0, 180);
        draw_text6x8_bgra(img, stride, w, h, tx, ty, buf, 255, 255, 255, 255);
    }

    if (globalSettings.enableMinValueDisplay)
    {
        float px = finite_or(cameraUtils.lastResult.MinTemperaturePoint.positionX, 0.5f);
        float py = finite_or(cameraUtils.lastResult.MinTemperaturePoint.positionY, 0.5f);
        px = clamp_f(px, 0.0f, 1.0f);
        py = clamp_f(py, 0.0f, 1.0f);

        int cx = (int)lroundf(px * (float)(w - 1));
        int cy = (int)lroundf(py * (float)(h - 1));
        draw_crosshair_bgra(img, stride, w, h, cx, cy, 0, 255, 255);

        float t = finite_or(cameraUtils.lastResult.minTemperature, 0.0f);
        snprintf(buf, sizeof(buf), "%.1f", t);
        int tx = cx + 8;
        int ty = cy - 4;
        int tw = (int)strlen(buf) * 6;
        
        fill_round_rect_bgra(img, stride, w, h, tx - 2, ty - 2, tw + 4, 12, 3, 0, 0, 0, 140);
        
        draw_text6x8_bgra(img, stride, w, h, tx + 1, ty + 1, buf, 0, 0, 0, 180);
        draw_text6x8_bgra(img, stride, w, h, tx, ty, buf, 255, 255, 255, 255);
    }
}





typedef struct
{
    uint8_t *bgra;
    int stride;
    int width;
    int height;
    uint64_t ts_ms;
} frame_item_t;

static pthread_t g_thread;
static pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_cv = PTHREAD_COND_INITIALIZER;

static int g_running = 0;
static int g_stop_req = 0;
static int g_recording = 0;

static frame_item_t g_queue[2];
static int g_q_head = 0;
static int g_q_tail = 0;
static int g_q_count = 0;

static AVFormatContext *g_ofmt = NULL;
static AVCodecContext *g_enc = NULL;
static AVStream *g_vst = NULL;
static struct SwsContext *g_sws = NULL;
static AVFrame *g_yuv = NULL;
static int g_fps = 25;
static int64_t g_pts = 0;
static int g_open_w = 0;
static int g_open_h = 0;


static FILE *g_mjpeg_fp = NULL;
static int g_use_tinyjpeg = 0;
static uint8_t *g_rgba_tmp = NULL;
static size_t g_rgba_tmp_bytes = 0;

static void tje_write_cb(void *context, void *data, int size)
{
    FILE *fp = (FILE *)context;
    if (!fp || !data || size <= 0) return;
    fwrite(data, 1, (size_t)size, fp);
}

static void ff_free_all()
{
    if (g_yuv) { av_frame_free(&g_yuv); g_yuv = NULL; }
    if (g_sws) { sws_freeContext(g_sws); g_sws = NULL; }
    if (g_enc) { avcodec_free_context(&g_enc); g_enc = NULL; }

    if (g_ofmt)
    {
        if (!(g_ofmt->oformat->flags & AVFMT_NOFILE) && g_ofmt->pb)
            avio_closep(&g_ofmt->pb);
        avformat_free_context(g_ofmt);
        g_ofmt = NULL;
    }
    g_vst = NULL;

    
    if (g_mjpeg_fp) { fclose(g_mjpeg_fp); g_mjpeg_fp = NULL; }
    if (g_rgba_tmp) { free(g_rgba_tmp); g_rgba_tmp = NULL; g_rgba_tmp_bytes = 0; }
    g_use_tinyjpeg = 0;
}

static void log_av_err(const char *what, int err)
{
    (void)what;
    (void)err;
}

static bool str_endswith(const char *s, const char *suffix)
{
    if (!s || !suffix) return false;
    size_t sl = strlen(s);
    size_t su = strlen(suffix);
    if (su > sl) return false;
    return memcmp(s + (sl - su), suffix, su) == 0;
}

static const AVCodec *find_encoder_for_filename(const char *filename, bool *out_is_mjpeg)
{
    bool is_mjpeg = false;
    if (filename)
    {
        if (str_endswith(filename, ".mjpeg") || str_endswith(filename, ".mjpg"))
            is_mjpeg = true;
    }
    if (out_is_mjpeg) *out_is_mjpeg = is_mjpeg;

    if (!is_mjpeg)
        return NULL;

    const AVCodec *codec = avcodec_find_encoder_by_name("mjpeg");
    if (codec) return codec;
    return avcodec_find_encoder(AV_CODEC_ID_MJPEG);
}

static bool ff_open(const char *filename, int width, int height, int fps)
{
    g_fps = (fps > 0) ? fps : 25;
    g_pts = 0;
    g_open_w = width;
    g_open_h = height;

    const bool want_mjpeg = filename && (str_endswith(filename, ".mjpeg") || str_endswith(filename, ".mjpg"));
    const bool force_ffmpeg_mjpeg = (access("/tmp/rec_force_ffmpeg_mjpeg", F_OK) == 0);

    
    
    if (want_mjpeg && !force_ffmpeg_mjpeg)
    {
        g_use_tinyjpeg = 1;
        g_mjpeg_fp = fopen(filename, "wb");
        if (!g_mjpeg_fp)
        {
            g_use_tinyjpeg = 0;
            return false;
        }
        return true;
    }

    
    const char *fmt_name = NULL;
    if (filename)
    {
        if (want_mjpeg)
            fmt_name = "mjpeg";
    }

    avformat_alloc_output_context2(&g_ofmt, NULL, fmt_name, filename);
    if (!g_ofmt)
    {
        return false;
    }

    bool is_mjpeg = false;
    const AVCodec *codec = find_encoder_for_filename(filename, &is_mjpeg);
    if (!codec)
    {
        return false;
    }

    g_vst = avformat_new_stream(g_ofmt, NULL);
    if (!g_vst)
    {
        return false;
    }

    g_enc = avcodec_alloc_context3(codec);
    if (!g_enc)
    {
        return false;
    }

    g_enc->codec_id = codec->id;
    g_enc->codec_type = AVMEDIA_TYPE_VIDEO;
    g_enc->width = width;
    g_enc->height = height;
    g_enc->time_base = AVRational{1, g_fps};
    g_enc->framerate = AVRational{g_fps, 1};
    g_enc->max_b_frames = 0;

    if (is_mjpeg)
    {
        g_enc->gop_size = 1;
        
        g_enc->color_range = AVCOL_RANGE_JPEG;

        
        g_enc->pix_fmt = AV_PIX_FMT_YUVJ420P;
        if (codec->pix_fmts)
        {
            
            const enum AVPixelFormat *p = codec->pix_fmts;
            bool ok = false;
            for (; *p != AV_PIX_FMT_NONE; ++p)
            {
                if (*p == AV_PIX_FMT_YUVJ420P) { ok = true; break; }
            }
            g_enc->pix_fmt = ok ? AV_PIX_FMT_YUVJ420P : codec->pix_fmts[0];
        }

        
        g_enc->flags |= AV_CODEC_FLAG_QSCALE;
        g_enc->global_quality = FF_QP2LAMBDA * 2;
        g_enc->qmin = 2;
        g_enc->qmax = 2;
    }
    else
    {
        g_enc->gop_size = g_fps * 2;
        g_enc->bit_rate = 900000; 

        
        g_enc->pix_fmt = AV_PIX_FMT_NV12;
        if (codec->pix_fmts)
        {
            const enum AVPixelFormat *p = codec->pix_fmts;
            bool ok = false;
            for (; *p != AV_PIX_FMT_NONE; ++p)
            {
                if (*p == AV_PIX_FMT_NV12) { ok = true; break; }
            }
            if (!ok) g_enc->pix_fmt = codec->pix_fmts[0];
        }
    }

    if (g_ofmt->oformat->flags & AVFMT_GLOBALHEADER)
        g_enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    AVDictionary *opts = NULL;

    int ret = avcodec_open2(g_enc, codec, &opts);
    av_dict_free(&opts);
    if (ret < 0)
    {
        return false;
    }

    ret = avcodec_parameters_from_context(g_vst->codecpar, g_enc);
    if (ret < 0)
    {
        return false;
    }
    g_vst->time_base = g_enc->time_base;

    if (!(g_ofmt->oformat->flags & AVFMT_NOFILE))
    {
        ret = avio_open(&g_ofmt->pb, filename, AVIO_FLAG_WRITE);
        if (ret < 0)
        {
            log_av_err("avio_open", ret);
            return false;
        }
    }

    ret = avformat_write_header(g_ofmt, NULL);
    if (ret < 0)
    {
        log_av_err("avformat_write_header", ret);
        return false;
    }

    g_sws = sws_getContext(width, height, AV_PIX_FMT_BGRA,
                           width, height, g_enc->pix_fmt,
                           SWS_FAST_BILINEAR, NULL, NULL, NULL);
    if (!g_sws)
    {
        return false;
    }

    g_yuv = av_frame_alloc();
    g_yuv->format = g_enc->pix_fmt;
    g_yuv->width = width;
    g_yuv->height = height;
    ret = av_frame_get_buffer(g_yuv, 32);
    if (ret < 0)
    {
        return false;
    }

    return true;
}

static void ff_write_one_bgra(uint8_t *bgra, int stride, int width, int height, graph_state_t *gs)
{
    const bool no_overlay = (access("/tmp/rec_no_overlay", F_OK) == 0) || (!globalSettings.preserveUI);

    const bool no_encode  = (access("/tmp/rec_no_encode", F_OK) == 0);
    const bool no_write   = (access("/tmp/rec_no_write", F_OK) == 0);

    const bool skip_sws  = (access("/tmp/rec_skip_sws", F_OK) == 0);
    const bool skip_send = (access("/tmp/rec_skip_send", F_OK) == 0);
    const bool skip_recv = (access("/tmp/rec_skip_recv", F_OK) == 0);

    const bool use_tinyjpeg = (g_use_tinyjpeg != 0);

    
    if (!no_overlay)
    {
        const bool force_sw = (access("/tmp/rec_sw_overlay", F_OK) == 0);

        
        bool ok = false;
        if (!force_sw)
        {
            ok = overlay_lvgl_burnin_bgra(bgra, stride, width, height);
        }

        
        if (!ok)
        {
            overlay_graph_bgra(bgra, stride, width, height, gs);
            overlay_crosshairs_bgra(bgra, stride, width, height);
        }
    }

    
    if (use_tinyjpeg)
    {
        if (!g_mjpeg_fp) return;

        const size_t need = (size_t)width * (size_t)height * 4u;
        if (!g_rgba_tmp || g_rgba_tmp_bytes < need)
        {
            uint8_t *p = (uint8_t *)realloc(g_rgba_tmp, need);
            if (!p) return;
            g_rgba_tmp = p;
            g_rgba_tmp_bytes = need;
        }

        
        
        
        const uint8_t *srcrow = bgra;
        for (int y = 0; y < height; ++y)
        {
            const uint8_t *s = srcrow;
            uint8_t *d = g_rgba_tmp + (size_t)y * (size_t)width * 4u;
            for (int x = 0; x < width; ++x)
            {
                d[0] = s[0]; 
                d[1] = s[1]; 
                d[2] = s[2]; 
                d[3] = 255;  
                s += 4;
                d += 4;
            }
            srcrow += stride;
        }

        
        tje_encode_with_func(tje_write_cb, g_mjpeg_fp, 3, width, height, 4, g_rgba_tmp);
        return;
    }

    if (no_encode)
        return;

    
    const uint8_t *src[4] = {bgra, NULL, NULL, NULL};
    int src_stride[4] = {stride, 0, 0, 0};
    if (av_frame_make_writable(g_yuv) < 0)
    {
        return;
    }

    if (skip_sws)
    {
        const int fmt = g_enc ? (int)g_enc->pix_fmt : (int)g_yuv->format;
        if (fmt == (int)AV_PIX_FMT_YUVJ420P || fmt == (int)AV_PIX_FMT_YUV420P)
        {
            for (int y = 0; y < g_yuv->height; ++y)
                memset(g_yuv->data[0] + (size_t)y * (size_t)g_yuv->linesize[0], 16, (size_t)g_yuv->width);
            for (int y = 0; y < g_yuv->height / 2; ++y)
            {
                memset(g_yuv->data[1] + (size_t)y * (size_t)g_yuv->linesize[1], 128, (size_t)g_yuv->width / 2);
                memset(g_yuv->data[2] + (size_t)y * (size_t)g_yuv->linesize[2], 128, (size_t)g_yuv->width / 2);
            }
        }
        else if (fmt == (int)AV_PIX_FMT_NV12)
        {
            for (int y = 0; y < g_yuv->height; ++y)
                memset(g_yuv->data[0] + (size_t)y * (size_t)g_yuv->linesize[0], 16, (size_t)g_yuv->width);
            for (int y = 0; y < g_yuv->height / 2; ++y)
                memset(g_yuv->data[1] + (size_t)y * (size_t)g_yuv->linesize[1], 128, (size_t)g_yuv->width);
        }
        else
        {
            
            for (int y = 0; y < g_yuv->height; ++y)
                memset(g_yuv->data[0] + (size_t)y * (size_t)g_yuv->linesize[0], 0, (size_t)g_yuv->linesize[0]);
        }
    }
    else
    {
        sws_scale(g_sws, src, src_stride, 0, height, g_yuv->data, g_yuv->linesize);
    }

    g_yuv->pts = g_pts++;

    if (skip_send)
    {
        return;
    }

    int ret = avcodec_send_frame(g_enc, g_yuv);
    if (ret < 0)
    {
        return;
    }

    if (skip_recv)
    {
        return;
    }

    AVPacket *pkt = av_packet_alloc();
    if (!pkt) return;
    while (ret >= 0)
    {
        ret = avcodec_receive_packet(g_enc, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) break;
        if (ret < 0)
        {
            break;
        }

        pkt->stream_index = g_vst->index;
        av_packet_rescale_ts(pkt, g_enc->time_base, g_vst->time_base);

        if (!no_write)
        {
            av_interleaved_write_frame(g_ofmt, pkt);
        }

        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);
}

static void ff_close()
{
    
    if (g_use_tinyjpeg)
    {
        ff_free_all();
        return;
    }

    if (!g_ofmt || !g_enc) { ff_free_all(); return; }

    
    avcodec_send_frame(g_enc, NULL);
    AVPacket *pkt = av_packet_alloc();
    while (1)
    {
        int ret = avcodec_receive_packet(g_enc, pkt);
        if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN)) break;
        if (ret < 0) break;
        pkt->stream_index = g_vst->index;
        av_packet_rescale_ts(pkt, g_enc->time_base, g_vst->time_base);
        av_interleaved_write_frame(g_ofmt, pkt);
        av_packet_unref(pkt);
    }
    av_packet_free(&pkt);

    av_write_trailer(g_ofmt);
    ff_free_all();
}

static void queue_clear()
{
    for (int i = 0; i < 2; ++i)
    {
        if (g_queue[i].bgra) free(g_queue[i].bgra);
        memset(&g_queue[i], 0, sizeof(g_queue[i]));
    }
    g_q_head = g_q_tail = g_q_count = 0;
}

static void *thread_func(void *)
{
    graph_state_t gs = {};

    while (1)
    {
        pthread_mutex_lock(&g_mtx);
        while (!g_stop_req && g_q_count == 0)
            pthread_cond_wait(&g_cv, &g_mtx);

        if (g_stop_req && g_q_count == 0)
        {
            pthread_mutex_unlock(&g_mtx);
            break;
        }

        frame_item_t item = g_queue[g_q_head];
        memset(&g_queue[g_q_head], 0, sizeof(frame_item_t));
        g_q_head = (g_q_head + 1) % 2;
        g_q_count--;
        pthread_mutex_unlock(&g_mtx);

        const bool dry_run = (access("/tmp/rec_dryrun", F_OK) == 0);

        if (item.bgra && dry_run)
        {
            
        }
        else if (item.bgra && (g_use_tinyjpeg || (g_ofmt && g_enc)))
        {
            ff_write_one_bgra(item.bgra, item.stride, item.width, item.height, &gs);
        }

        if (item.bgra) free(item.bgra);
    }

    if (gs.min_buf) free(gs.min_buf);
    if (gs.max_buf) free(gs.max_buf);
    return NULL;
}

bool recorder_start(const char *filename, int width, int height, int fps)
{
    const bool dry_run = (access("/tmp/rec_dryrun", F_OK) == 0);

    pthread_mutex_lock(&g_mtx);
    if (g_recording)
    {
        pthread_mutex_unlock(&g_mtx);
        return false;
    }

    queue_clear();
    g_stop_req = 0;

    if (dry_run)
    {
        g_fps = (fps > 0) ? fps : 25;
        g_pts = 0;
        g_open_w = width;
        g_open_h = height;
    }
    else if (!ff_open(filename, width, height, fps))
    {
        
        ff_free_all();
        pthread_mutex_unlock(&g_mtx);
        return false;
    }

    g_recording = 1;
    if (!g_running)
    {
        g_running = 1;
        pthread_create(&g_thread, NULL, thread_func, NULL);
    }

    pthread_mutex_unlock(&g_mtx);
    return true;
}

void recorder_stop()
{
    pthread_mutex_lock(&g_mtx);
    if (!g_recording)
    {
        pthread_mutex_unlock(&g_mtx);
        return;
    }
    g_recording = 0;

    
    g_stop_req = 1;
    pthread_cond_signal(&g_cv);
    pthread_mutex_unlock(&g_mtx);

    
    if (g_running)
    {
        pthread_join(g_thread, NULL);
        g_running = 0;
    }

    pthread_mutex_lock(&g_mtx);
    queue_clear();
    pthread_mutex_unlock(&g_mtx);

    ff_close();
}

bool recorder_is_recording()
{
    pthread_mutex_lock(&g_mtx);
    int r = g_recording;
    pthread_mutex_unlock(&g_mtx);
    return r != 0;
}

void recorder_submit_bgra(const uint8_t *bgra, int stride, int width, int height)
{
    const bool dry_run = (access("/tmp/rec_dryrun", F_OK) == 0);

    pthread_mutex_lock(&g_mtx);

    if (!g_recording || (!dry_run && !g_use_tinyjpeg && (!g_ofmt || !g_enc)))
    {
        pthread_mutex_unlock(&g_mtx);
        return;
    }

    
    if (width != g_open_w || height != g_open_h || width <= 0 || height <= 0)
    {
        pthread_mutex_unlock(&g_mtx);
        return;
    }

    
    if (g_q_count == 2)
    {
        frame_item_t *old = &g_queue[g_q_head];
        if (old->bgra) free(old->bgra);
        memset(old, 0, sizeof(*old));
        g_q_head = (g_q_head + 1) % 2;
        g_q_count--;
    }

    
    const int dst_stride = width * 4;
    const int src_stride = (stride > 0) ? stride : dst_stride;
    if (src_stride < dst_stride)
    {
        pthread_mutex_unlock(&g_mtx);
        return;
    }

    frame_item_t *it = &g_queue[g_q_tail];
    memset(it, 0, sizeof(*it));
    it->stride = dst_stride;
    it->width = width;
    it->height = height;
    it->ts_ms = now_ms();

    size_t bytes = (size_t)dst_stride * (size_t)height;
    it->bgra = (uint8_t *)malloc(bytes);
    if (!it->bgra)
    {
        pthread_mutex_unlock(&g_mtx);
        return;
    }

    
    for (int y = 0; y < height; ++y)
    {
        memcpy(it->bgra + (size_t)y * (size_t)dst_stride, bgra + (size_t)y * (size_t)src_stride, (size_t)dst_stride);
    }

    g_q_tail = (g_q_tail + 1) % 2;
    g_q_count++;

    pthread_cond_signal(&g_cv);
    pthread_mutex_unlock(&g_mtx);
}
