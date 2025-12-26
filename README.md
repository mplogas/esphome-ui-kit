# ESPHome LVGL Abstraction for SenseCAP Indicator

This project provides a modular configuration framework for building LVGL-based user interfaces on the Seeed SenseCAP Indicator using ESPHome. It abstracts hardware complexity and provides a reusable component system for rapid UI development.

## Directory Structure

- `hardware/`: Device-specific configurations and screen dimensions.
- `layout/`: UI structural components (Tabview, Grid containers).
- `tabs/`: Individual screen definitions.
- `theme/`: Global variables for colors, fonts, and layout defaults.
- `widgets/`: Reusable UI elements (Clock, Weather, Buttons).

## Core Concepts

### Hardware Abstraction
The `hardware/sensecap-indicator.yaml` file contains the complete setup for the ESP32-S3, PSRAM, MIPI RGB display, and FT5x06 touchscreen.

### Grid-based Layout
Each tab uses a `grid_container.yaml` which implements LVGL's grid layout. Column and row specifications are passed via substitutions, allowing for responsive or fixed-size layouts.

### Reusable Widgets
Widgets are defined as YAML snippets that accept variables for:
- `widget_id`: Unique identifier for the component.
- `grid_col_pos` / `grid_row_pos`: Position within the parent grid.
- `grid_x_align` / `grid_y_align`: Alignment within the grid cell.
- `widget_clickable`: Boolean to enable touch events.
- `widget_on_click`: C++ lambda snippet for touch actions.

### Theme System
The `theme/defaults.yaml` file centralizes all UI parameters, including:
- Screen dimensions (`screen_width`, `screen_height`).
- Color palette (hex strings).
- Grid spacing and padding.
- Default font IDs.
- Idle timeout duration (in seconds).

## Features

- **Touch-to-Wake**: Automatically resumes LVGL and enables the backlight on touch release after an idle period.
- **Idle Management**: Configurable timeout that pauses LVGL and disables the backlight to reduce power consumption.
- **Clickable Cards**: Any widget container can be made clickable, with visual feedback defined in the theme styles.

## Usage

1. Define global theme parameters in `theme/defaults.yaml`.
2. Create individual tab files in `tabs/` (e.g., `home.yaml`).
3. List active tabs in `tabs/main_tabs.yaml`.
4. Compile and flash using `main-dashboard.yaml` as the entry point.

```bash
esphome run main-dashboard.yaml
```
