#ifndef LVGL_CHART_H
#define LVGL_CHART_H

#include "lvgl.h"
#include <vector>

void init_lvgl_chart(lv_obj_t * parent, int w, int h, int x, int y, uint32_t color_hex) {
    lv_obj_t * chart = lv_chart_create(parent);
    lv_obj_set_size(chart, w, h);
    lv_obj_set_pos(chart, x, y);
    
    // Clean look
    lv_obj_set_style_bg_color(chart, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(chart, 0, LV_PART_MAIN); // Transparent bg
    lv_obj_set_style_border_width(chart, 0, LV_PART_MAIN);
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR); // No dots
    
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(chart, 0, 0); // No grid lines
    
    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_color_hex(color_hex), LV_CHART_AXIS_PRIMARY_Y);
    
    // Store the series pointer in the chart's user_data for later retrieval
    chart->user_data = ser;
}

void update_lvgl_chart(lv_obj_t * chart, const std::vector<float>* values, float min_val, float max_val) {
    if (chart == nullptr || values == nullptr) return;
    lv_chart_series_t * ser = (lv_chart_series_t *)chart->user_data;
    if (ser == nullptr) return;

    // Auto-scale if min/max are same
    if (min_val == max_val) {
        min_val = 0;
        max_val = 100;
    }

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, (int32_t)min_val, (int32_t)max_val);
    lv_chart_set_point_count(chart, values->size());
    
    // Clear and refill to ensure sync with vector
    lv_chart_set_all_value(chart, ser, LV_CHART_POINT_NONE);
    for (float val : *values) {
        lv_chart_set_next_value(chart, ser, (int32_t)val);
    }
    lv_chart_refresh(chart);
}

#endif
