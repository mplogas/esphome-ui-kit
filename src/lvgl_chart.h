#ifndef LVGL_CHART_H
#define LVGL_CHART_H

#include "lvgl.h"
#include <vector>

static lv_obj_t * chart_obj = nullptr;
static lv_chart_series_t * chart_ser = nullptr;

void init_lvgl_chart(lv_obj_t * parent, int w, int h, int x, int y, uint32_t color_hex) {
    if (chart_obj != nullptr) return;

    chart_obj = lv_chart_create(parent);
    lv_obj_set_size(chart_obj, w, h);
    lv_obj_set_pos(chart_obj, x, y);
    
    // Clean look
    lv_obj_set_style_bg_color(chart_obj, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(chart_obj, 0, LV_PART_MAIN); // Transparent bg
    lv_obj_set_style_border_width(chart_obj, 0, LV_PART_MAIN);
    lv_obj_set_style_size(chart_obj, 0, LV_PART_INDICATOR); // No dots
    
    lv_chart_set_type(chart_obj, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(chart_obj, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(chart_obj, 0, 0); // No grid lines
    
    chart_ser = lv_chart_add_series(chart_obj, lv_color_hex(color_hex), LV_CHART_AXIS_PRIMARY_Y);
}

void update_lvgl_chart(const std::vector<float>& values, float min_val, float max_val) {
    if (chart_obj == nullptr || chart_ser == nullptr) return;

    // Auto-scale if min/max are same
    if (min_val == max_val) {
        min_val = 0;
        max_val = 100;
    }

    lv_chart_set_range(chart_obj, LV_CHART_AXIS_PRIMARY_Y, (int32_t)min_val, (int32_t)max_val);
    lv_chart_set_point_count(chart_obj, values.size());
    
    // Clear and refill to ensure sync with vector
    lv_chart_set_all_value(chart_obj, chart_ser, LV_CHART_POINT_NONE);
    for (float val : values) {
        lv_chart_set_next_value(chart_obj, chart_ser, (int32_t)val);
    }
    lv_chart_refresh(chart_obj);
}

#endif
