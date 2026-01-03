# ESPHome LVGL Abstraction

This project provides a modular configuration framework for building LVGL-based user interfaces. It abstracts hardware complexity and provides a reusable component system for rapid UI development. The demo hardware is a Seeedstudio SenseCAP Indicator (D1x), but it should work with any display.

## Directory Structure

- `hardware/`: Device-specific configurations and screen dimensions.
- `layout/`: UI structural components (Tabview, Grid containers).
- `tabs/`: Individual screen definitions.
- `templates/`: Reusable logic, sensors, and UI overlays.
- `theme/`: Global variables for colors, fonts, and layout defaults.
- `widgets/`: Reusable UI elements (Clock, Weather, Buttons).

## Core Concepts

### Hardware Abstraction
The `hardware/sensecap-indicator.yaml` file contains the complete setup for the ESP32-S3, PSRAM, MIPI RGB display, and FT5x06 touchscreen. Place your hardware template here and reference it from you ESPHome files.

### Logic & Packages
The configuration uses ESPHome `packages` to separate concerns:
- `hardware`: Device drivers and display setup.
- `common_logic`: Centralized sensors, scripts, and globals (e.g., weather logic, light control state).

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

- **Idle Management**: Configurable timeout that pauses LVGL and disables the backlight to reduce power consumption.
- **Clickable Cards**: Any widget container can be made clickable, with visual feedback defined in the theme styles.
- **Reusable Widgets** : Widgets are defined once and reused across multiple dashboards or devices.
- **Weather Integration**: Built-in support for weather and rain sensors via `common_logic.yaml`.

## Usage

1. Copy `main-dashboard.yaml` to a new file (e.g., `my-device.yaml`).
2. Update the `substitutions` block with your Home Assistant entity IDs (lights, scenes, weather).
3. Compile and flash:

```bash
esphome run my-device.yaml
```
