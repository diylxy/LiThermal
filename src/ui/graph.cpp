#include <my_main.h>

#include <math.h>
typedef uint16_t (*MyChartGetValueCallback)(int id); // 获取数据的回调函数，每隔data_interval时间调用一次，非阻塞
class MyChart
{
public:
    lv_obj_t *_chart_widget = NULL;
    lv_chart_series_t *_series = NULL;
    lv_chart_series_t *_series2 = NULL;
    uint16_t _width;
    uint16_t _height;
    uint16_t _data_buffer[2];      //[0]为上一次数据，[1]为当前数据, 0..1000
    uint16_t _data_buffer2[2];     //[0]为上一次数据，[1]为当前数据, 0..1000
    uint16_t _data_weight_current; // 记录距离上一次获取数据以来的图表刷新次数
    uint16_t _data_weight_max;     // 记录插值次数
    uint16_t _data_count;
    uint16_t _data_interval;
    MyChartGetValueCallback _callback;
    lv_timer_t *_timer = NULL;
    lv_obj_t *label = NULL;
    float label_factor = 1.0;
    lv_obj_t *lbl_title = NULL;
    uint16_t _update_interval = 20;
    void init(lv_obj_t *parent, uint16_t width, uint16_t height, uint16_t data_interval, uint16_t default_value, MyChartGetValueCallback callback, uint16_t update_interval = 20);
    void destroy();
    void showLabel(bool show)
    {
        if (show)
        {
            if (label)
                goto end;
            label = lv_label_create(_chart_widget);
            lv_obj_set_style_bg_color(label, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
            lv_obj_set_style_bg_opa(label, LV_OPA_30, 0);
            lv_obj_set_style_radius(label, 3, 0);
            lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -5, 0);
        }
        else
        {
            if (!label)
                goto end;
            lv_obj_del(label);
            label = NULL;
        }
    end:
        return;
    }
    void showTitle(const char *title)
    {
        if (title)
        {
            if (lbl_title)
            {
                lv_label_set_text(lbl_title, title);
                goto end;
            }
            lbl_title = lv_label_create(_chart_widget);

            lv_obj_set_style_text_font(lbl_title, &ui_font_chinese16, 0);
            lv_label_set_text(lbl_title, title);
            lv_obj_set_style_bg_color(lbl_title, lv_palette_main(LV_PALETTE_BLUE_GREY), 0);
            lv_obj_set_style_bg_opa(lbl_title, LV_OPA_30, 0);
            lv_obj_set_style_radius(lbl_title, 3, 0);
            lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 0);
        }
        else
        {
            if (lbl_title)
            {
                lv_obj_del(lbl_title);
                lbl_title = NULL;
            }
        }
    end:
        return;
    }
    uint16_t cosineInterpolation(uint16_t data_buffer_ptr[2])
    {
        float result = 0.0;
        float point = (float)_data_weight_current / (float)_data_weight_max;

        // 计算权重
        float weight = (1 - cos((point)*M_PI)) / 2;

        // 执行余弦插值计算
        result = (1 - weight) * data_buffer_ptr[0] + weight * data_buffer_ptr[1];

        return result;
    }
};
void MyChart::init(lv_obj_t *parent, uint16_t width, uint16_t height, uint16_t data_interval, uint16_t default_value, MyChartGetValueCallback callback, uint16_t update_interval)
{
    _update_interval = update_interval;
    _chart_widget = lv_chart_create(parent);
    lv_obj_set_size(_chart_widget, width, height);
    lv_chart_set_update_mode(_chart_widget, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_point_count(_chart_widget, width);
    lv_obj_set_style_line_width(_chart_widget, 1, LV_PART_ITEMS);
    lv_chart_set_range(_chart_widget, LV_CHART_AXIS_PRIMARY_Y, 0, 1000);
    lv_chart_set_range(_chart_widget, LV_CHART_AXIS_SECONDARY_Y, 0, 1000);
    lv_chart_set_type(_chart_widget, LV_CHART_TYPE_LINE);
    lv_obj_center(_chart_widget);
    lv_obj_set_style_bg_opa(_chart_widget, 0, 0);
    lv_obj_set_style_border_width(_chart_widget, 0, 0);

    _series = lv_chart_add_series(_chart_widget, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
    _series2 = lv_chart_add_series(_chart_widget, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
    lv_chart_set_all_value(_chart_widget, _series, default_value);
    lv_chart_set_all_value(_chart_widget, _series2, default_value);

    lv_obj_set_style_opa(_chart_widget, LV_OPA_0, LV_PART_INDICATOR);
    lv_obj_set_style_pad_all(_chart_widget, 0, 0);
    lv_obj_clear_flag(_chart_widget, LV_OBJ_FLAG_SCROLLABLE);
    _timer = lv_timer_create([](lv_timer_t *timer)
                             {
        MyChart *chart_ptr = (MyChart *)timer->user_data;
        if(chart_ptr->_data_weight_current >= chart_ptr->_data_weight_max)
        {
            chart_ptr->_data_weight_current = 0;
            if(chart_ptr->_callback != NULL)
            {
                chart_ptr->_data_buffer[0] = chart_ptr->_data_buffer[1];
                chart_ptr->_data_buffer[1] = chart_ptr->_callback(GRAPH_DATA_SOURCE_MIN);
                chart_ptr->_data_buffer2[0] = chart_ptr->_data_buffer2[1];
                chart_ptr->_data_buffer2[1] = chart_ptr->_callback(GRAPH_DATA_SOURCE_MAX);
            }
        }
        lv_obj_move_foreground(lv_obj_get_parent(chart_ptr->_chart_widget));
        if (chart_ptr->_data_interval == chart_ptr->_update_interval)
        {
            lv_chart_set_next_value(chart_ptr->_chart_widget, chart_ptr->_series, chart_ptr->_data_buffer[1]);
            lv_chart_set_next_value(chart_ptr->_chart_widget, chart_ptr->_series2, chart_ptr->_data_buffer2[1]);
        }
        else
        {
            float next = chart_ptr->cosineInterpolation(chart_ptr->_data_buffer);
            float next2 = chart_ptr->cosineInterpolation(chart_ptr->_data_buffer2);
            lv_chart_set_next_value(chart_ptr->_chart_widget, chart_ptr->_series, next);
            lv_chart_set_next_value(chart_ptr->_chart_widget, chart_ptr->_series2, next2);
        }

        lv_coord_t *ser = lv_chart_get_y_array(chart_ptr->_chart_widget, chart_ptr->_series);
        lv_coord_t *ser2 = lv_chart_get_y_array(chart_ptr->_chart_widget, chart_ptr->_series2);
        int size = lv_chart_get_point_count(chart_ptr->_chart_widget);
        lv_coord_t max_num = INT16_MIN, min_num = INT16_MAX;
        for (int i = 0; i < size; ++i)
        {
            if (ser[i] > max_num)
                max_num = ser[i];
            if (ser[i] < min_num)
                min_num = ser[i];
            if (ser2[i] > max_num)
                max_num = ser2[i];
            if (ser2[i] < min_num)
                min_num = ser2[i];
        }
        lv_chart_set_range(chart_ptr->_chart_widget, LV_CHART_AXIS_PRIMARY_Y, min_num - 50, max_num + 50);
        lv_chart_set_range(chart_ptr->_chart_widget, LV_CHART_AXIS_SECONDARY_Y, min_num - 50, max_num + 50);

        // if(chart_ptr->label != NULL)
        // {
        //     static char buf[16];
        //     sprintf(buf, "%.1f", next * chart_ptr->label_factor);
        //     lv_label_set_text(chart_ptr->label, buf);
        //     float pos = (max_num - next) * chart_ptr->_height / (max_num - min_num);
        //     if(pos < 2)
        //         pos = 2;
        //     if(pos + lv_obj_get_height(chart_ptr->label) >= chart_ptr->_height - 5)
        //         pos -= lv_obj_get_height(chart_ptr->label) + 5;
        //     lv_obj_set_y(chart_ptr->label, (uint16_t)(pos));
        // }
        chart_ptr->_data_weight_current++; },
                             _update_interval, this);
    _width = width;
    _height = height;
    _callback = callback;
    _data_weight_current = 0;
    _data_interval = data_interval;
    _data_count = width * _update_interval / data_interval;
    _data_weight_max = width / _data_count;
    _data_buffer[0] = default_value;
    _data_buffer[1] = default_value;
    label = NULL;
    lbl_title = NULL;
    label_factor = 1.0;
}

void MyChart::destroy()
{
    if (_timer)
        lv_timer_del(_timer);
    if (_chart_widget)
        lv_obj_del(_chart_widget);
    _timer = NULL;
    _chart_widget = NULL;
    label = NULL;
}

MyChart chart;
MyCard card;
#define CARD_SIZE_SMALL_WIDTH 120
#define CARD_SIZE_LARGE_WIDTH 240
#define CARD_SIZE_HEIGHT 120

#define GRAPH_SIZE_SMALL false
#define GRAPH_SIZE_LAEGE true
static int current_pos = 0;
static bool is_showed = false;
static int graph_hide_pos[4][2] = {
    {-260, 10},
    {330, 10},
    {330, 240 - CARD_SIZE_HEIGHT - 10},
    {-260, 240 - CARD_SIZE_HEIGHT - 10},
};

static void setGraphPosition(int new_pos)
{
    int x, y;
    if (globalSettings.graphSize == GRAPH_SIZE_SMALL)
    {
        if (new_pos == 1 || new_pos == 2)
            x = 320 - CARD_SIZE_SMALL_WIDTH - 10;
        else
            x = 10;
    }
    else
    {
        if (new_pos == 1 || new_pos == 2)
            x = 320 - CARD_SIZE_LARGE_WIDTH - 10;
        else
            x = 10;
    }
    if (new_pos == 2 || new_pos == 3)
        y = 240 - CARD_SIZE_HEIGHT - 10;
    else
        y = 10;
    card.move(x, y);
}

static void hideGraph()
{
    card.move(graph_hide_pos[current_pos][0], graph_hide_pos[current_pos][1]);
    lv_obj_fade_out(card.obj, 500, 0);
    is_showed = false;
}

static void showGraph()
{
    card.move(graph_hide_pos[current_pos][0], graph_hide_pos[current_pos][1], false);
    setGraphPosition(current_pos);
    lv_obj_fade_in(card.obj, 500, 0);
    is_showed = true;
}

static uint16_t getGraphData(int id)
{
    switch (id)
    {
    case GRAPH_DATA_SOURCE_MAX:
        return cameraUtils.lastResult.maxTemperature * 100;
        break;
    case GRAPH_DATA_SOURCE_MIN:
        return cameraUtils.lastResult.minTemperature * 100;
        break;
    default:
        break;
    }
    return 0;
}

static void resizeGraph(bool new_graph_size)
{
    int w;
    if (new_graph_size == GRAPH_SIZE_LAEGE)
        w = CARD_SIZE_LARGE_WIDTH;
    else
        w = CARD_SIZE_SMALL_WIDTH;
    lv_anim_size(card.obj, w, CARD_SIZE_HEIGHT);
    lv_anim_size(chart._chart_widget, w, CARD_SIZE_HEIGHT);
    chart._width = w;
}
const int graph_data_interval_table[7] = {
    40,
    100,
    500,
    1000,
    2000,
    5000,
    10000,
};
void widget_graph_updateSettings()
{
    resizeGraph(globalSettings.graphSize);
    if (is_showed == false && globalSettings.enableGraph)
        showGraph();
    else if (is_showed == true && globalSettings.enableGraph == 0)
        hideGraph();
    chart._data_interval = graph_data_interval_table[globalSettings.graphRefreshInterval];
    chart._update_interval = graph_data_interval_table[globalSettings.graphRefreshInterval];
    chart._data_count = chart._width * chart._update_interval / chart._data_interval;
    chart._data_weight_max = chart._width / chart._data_count;
    lv_timer_set_period(chart._timer, graph_data_interval_table[globalSettings.graphRefreshInterval]);
    if (is_showed)
        setGraphPosition(globalSettings.graphPos);
}

void widget_graph_create()
{
    card.create(lv_scr_act(), 10, 10, CARD_SIZE_SMALL_WIDTH, CARD_SIZE_HEIGHT);
    lv_obj_set_style_pad_all(card.obj, 0, 0);
    lv_obj_set_style_bg_opa(card.obj, 128, 0);
    lv_obj_clear_flag(card.obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_clip_corner(card.obj, true, 0);
    chart.init(card.obj, CARD_SIZE_LARGE_WIDTH, CARD_SIZE_HEIGHT, graph_data_interval_table[globalSettings.graphRefreshInterval], 0, getGraphData, graph_data_interval_table[globalSettings.graphRefreshInterval]);
    lv_obj_set_size(chart._chart_widget, CARD_SIZE_SMALL_WIDTH, CARD_SIZE_HEIGHT);
    card.move(graph_hide_pos[current_pos][0], graph_hide_pos[current_pos][1], false);
    card.show(CARD_ANIM_NONE);
    chart.label_factor = 0.01;
}
