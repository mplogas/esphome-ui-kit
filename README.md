# ESPHome UI Kit - A LVGL Abstraction

A modular framework for building LVGL-based user interfaces in ESPHome. It abstracts hardware complexity and provides a reusable component system. Designed for the Seeedstudio SenseCAP Indicator (D1x) but adaptable to any display.

## Directory Structure

- `hardware/`: Device-specific configurations.
- `templates/`: Reusable logic and UI components.
    - `core/`: UI structural components.
    - `devices/`: Virtual device definitions.
    - `layouts/`: Page layout templates.
    - `logic/`: Shared logic and packages.
    - `overlays/`: Contextual UI layers.
    - `sensors/`: Template sensors for UI feedback.
    - `tabs/`: Screen definitions.
    - `tiles/`: Interactive UI cards.
    - `widgets/`: Standalone UI elements.
- `theme/`: UI styling and variables.

## Configuration Files

- `main-dashboard.yaml`: Main entry point and dashboard configuration.
- `secrets.yaml`: WiFi and API credentials.
- `theme/defaults.yaml`: Global UI variables (colors, dimensions, spacing).
- `theme/style.yaml`: LVGL style definitions.
- `theme/fonts.yaml`: Font mappings.
- `theme/light_mapping.yaml`: Light-specific color presets.

## Core Concepts

### Hardware Abstraction
Hardware-specific setup (ESP32-S3, PSRAM, display, touchscreen) is isolated in `hardware/`. Reference these templates in your main configuration.

### Logic & Packages
Uses ESPHome `packages` to separate concerns:
- `hardware`: Drivers and display initialization.
- `logic/common`: Shared sensors, scripts, and global state.

### Grid-based Layout
Tabs use `grid_container.yaml` to implement LVGL's grid layout. Dimensions and positions are passed via substitutions.

### Reusable Widgets
Widgets and Tiles are YAML snippets accepting:
- `widget_id` / `id`: Unique component identifier.
- `grid_col_pos` / `grid_row_pos` (or `col` / `row`): Grid position.
- `grid_x_align` / `grid_y_align`: Cell alignment (defaults to `STRETCH`).
- `widget_clickable`: Enable/disable touch events.
- `widget_on_click`: C++ lambda for touch actions.

### Theme System
`theme/defaults.yaml` centralizes UI parameters:
- Screen dimensions.
- Color palette.
- Grid spacing and padding.
- Font IDs.
- Idle timeout.

## Features

- **Idle Management**: Configurable timeout to pause LVGL and disable backlight.
- **Clickable Cards**: Any widget container can be made clickable with theme-defined visual feedback.
- **Reusable Widgets**: Components defined once and reused across dashboards.
- **Weather Integration**: Support for weather and rain sensors via `templates/logic/common.yaml`.

## Usage

1. Copy `main-dashboard.yaml` to a new file (e.g., `my-device.yaml`).
2. Update the `substitutions` block with your Home Assistant entity IDs (lights, scenes, weather).
3. Compile and flash:

```bash
esphome run my-device.yaml
```
