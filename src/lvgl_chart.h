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
    lv_obj_set_style_line_width(chart, 2, LV_PART_ITEMS);
    lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR); 
    
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_div_line_count(chart, 0, 0); // No grid lines
    
    lv_chart_series_t * ser = lv_chart_add_series(chart, lv_color_hex(color_hex), LV_CHART_AXIS_PRIMARY_Y);
    
    // Store the series pointer in the chart's user_data for later retrieval
    chart->user_data = ser;
}

void update_lvgl_chart(lv_obj_t * chart, const std::vector<float>* values, float min_val, float max_val) {
    if (chart == nullptr || values == nullptr || values->empty()) return;
    
    // Validate user_data before casting to prevent dereferencing invalid pointers
    if (chart->user_data == nullptr) {
        ESP_LOGW("lvgl_chart", "Chart user_data is null, cannot update");
        return;
    }
    lv_chart_series_t * ser = static_cast<lv_chart_series_t*>(chart->user_data);

    // 1. Apply simple smoothing (Exponential Moving Average) for display
    // We don't modify the original vector, just what we send to the chart
    std::vector<int32_t> smoothed_points;
    smoothed_points.reserve(values->size()); // Prevent repeated reallocations
    float ema = values->front();
    float alpha = 0.02f; // Lower alpha for a more "curvy" look (more smoothing)

    for (float val : *values) {
        ema = (alpha * val) + (1.0f - alpha) * ema;
        smoothed_points.push_back((int32_t)ema);
    }

    // 2. Hysteresis for Y-axis scaling
    // We add a 10% buffer and round to nice numbers to prevent the graph from "jittering"
    float range = max_val - min_val;
    if (range < 1.0f) range = 1.0f;
    
    // Round min down and max up to nearest "nice" interval (e.g. 5 or 10)
    // This keeps the scale stable even if values fluctuate slightly
    float padding = range * 0.15f;
    int32_t final_min = (int32_t)(min_val - padding);
    int32_t final_max = (int32_t)(max_val + padding);

    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, final_min, final_max);
    lv_chart_set_point_count(chart, smoothed_points.size());
    
    // Clear and refill
    lv_chart_set_all_value(chart, ser, LV_CHART_POINT_NONE);
    for (int32_t val : smoothed_points) {
        lv_chart_set_next_value(chart, ser, val);
    }
    lv_chart_refresh(chart);
}

#endif
