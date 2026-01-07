# ESPHome UI Kit - A LVGL Abstraction

A modular framework for building LVGL-based user interfaces in ESPHome. It abstracts hardware complexity and provides a reusable component system. Designed for the Seeedstudio SenseCAP Indicator (D1x) but adaptable to any display.

## Disclaimer

The README is auto-updated by an LLM. Please verify all information in code or with `main-dashboard.yaml` example file before use. I have also added an `AGENTS.md` and an `INSTRUCTIONS.md`, this should make building widgets easier via code-gen.    

## Directory Structure

- `hardware/`: Device-specific configurations.
- `src/`: Custom C++ headers for advanced LVGL features (e.g., Charts).
- `templates/`: Reusable logic and UI components.
    - `core/`: UI structural components, globals, and the Data Bridge.
    - `devices/`: Virtual device definitions.
    - `layouts/`: Page layout templates.
    - `scripts/`: Shared scripts and packages.
    - `overlays/`: Contextual UI layers.
    - `tabs/`: Screen definitions.
    - `widgets/`: Standalone UI elements.
- `theme/`: UI styling and variables.

## Configuration Files

- `main-dashboard.yaml`: Main entry point and dashboard configuration.
- `templates/core/mapping-defaults.yaml`: View 1 (Mapping) fallbacks and state defaults.
- `secrets.yaml`: WiFi and API credentials.
- `theme/defaults.yaml`: Global UI variables (colors, dimensions, spacing).
- `theme/style.yaml`: LVGL style definitions.
- `templates/core/fonts.yaml`: Font mappings.

## Core Concepts

### The 3-View Architecture
The framework enforces a strict separation of concerns:
- **View 1: User Configuration**: Handled in `main-dashboard.yaml` (or `testing.yaml`) and `templates/core/mapping-defaults.yaml`. Maps HA entities to UI slots.
- **View 2: Design & Templates**: Located in `templates/widgets/`, and `theme/`. These are visual components that read from the Data Pool.
- **View 3: Core Logic (Data Bridge)**: Located in `templates/core/data_bridge.yaml` and `widget_logic.yaml`. This layer connects HA to the Data Pool (Globals) and triggers UI refreshes.

### Hardware Abstraction
Hardware-specific setup (ESP32-S3, PSRAM, display, touchscreen) is isolated in `hardware/`. Reference these templates in your main configuration.

### Logic & Packages
Uses ESPHome `packages` to separate concerns:
- `hardware`: Drivers and display initialization.
- `core/globals`: The "Data Pool" of global variables.
- `core/data_bridge`: Home Assistant entity connections to the Data Pool.
- `core/widget_logic`: Scripts that sync the Data Pool to LVGL objects.
- `scripts/common`: Shared UI bridge scripts and logic.

### Grid-based Layout
Tabs use `grid_container.yaml` to implement LVGL's grid layout. Dimensions and positions are passed via substitutions.

### Reusable Widgets
Widgets and Tiles are YAML snippets accepting:
- `id`: Unique component identifier.
- `grid_col_pos` / `grid_row_pos` (or `col` / `row`): Grid position.
- `grid_x_align` / `grid_y_align`: Cell alignment (defaults to `STRETCH`).
- `widget_clickable`: Enable/disable touch events.
- `widget_on_click`: C++ lambda for touch actions.

### Theme System
`theme/defaults.yaml` centralizes UI parameters:
- Screen dimensions.
- Color palette (with auto-generated variants).
- Grid spacing and padding.
- Font IDs.
- Idle timeout.

## Features

- **Decoupled Sync (Data Bridge)**: UI components don't talk directly to Home Assistant. They observe global variables, ensuring the UI remains responsive even if HA is offline.
- **Dynamic Styling**: Light tiles automatically tint their icons and backgrounds based on the RGB state of the Home Assistant entity.
- **Idle Management**: Configurable timeout to pause LVGL and disable backlight.
- **Clickable Cards**: Any widget container can be made clickable with theme-defined visual feedback.
- **Advanced Data Visualization**: Custom LVGL Chart widget for displaying sensor history with auto-scaling and value overlays.

## Usage

1. Copy `main-dashboard.yaml` to a new file (e.g., `my-device.yaml`).
2. Update the `substitutions` block with your Home Assistant entity IDs (lights, scenes, weather).
3. Compile and flash:

```bash
esphome run my-device.yaml
```

## Adding New Graphs

The framework supports multiple independent graph widgets. To add a new one:

1.  **Define a Data Buffer**: In [templates/core/globals.yaml](templates/core/globals.yaml), add a new `std::vector<float>` to store the sensor history.
    ```yaml
    - id: my_sensor_values
      type: std::vector<float>
    ```
2.  **Add the Widget**: In your tab definition (e.g., [templates/tabs/overview.yaml](templates/tabs/overview.yaml)), include the graph widget with a unique `widget_id`.
    ```yaml
    - <<: !include
        file: ../widgets/graph.yaml
        vars:
          widget_id: my_new_graph
          graph_title: "My Sensor"
          graph_unit: "unit"
          graph_color: "0xHEXCODE"
          grid_col_pos: 0
          grid_row_pos: 1
    ```
3.  **Initialize on Boot**: In [main-dashboard.yaml](main-dashboard.yaml), add the initialization script to the `on_boot` trigger.
    ```yaml
    - script.execute:
        id: init_graph
        container_obj: !lambda "return id(my_new_graph_chart_container);"
        color: 0xHEXCODE
    ```
4.  **Connect the Sensor**: In [templates/core/data_bridge.yaml](templates/core/data_bridge.yaml), update your sensor to push data to the buffer and trigger the UI update.
    ```yaml
    on_value:
      - lambda: |-
          id(my_sensor_values).push_back(x);
          if (id(my_sensor_values).size() > ${chart_max_points}) {
            id(my_sensor_values).erase(id(my_sensor_values).begin());
          }
      - script.execute:
          id: update_graph
          chart_obj: !lambda "return id(my_new_graph_chart_container);"
          label_obj: !lambda "return id(my_new_graph_value_label);"
          values: !lambda "return &id(my_sensor_values);"
          unit: "unit"
    ```
