# ESPHome UI Kit - Project Instructions

## Goal
A lightweight, 3-layer abstraction over ESPHome's LVGL feature. The goal is to quickly set up sophisticated UIs for various ESPHome devices with displays (e.g., SenseCap Indicator) while maintaining a strict separation between configuration, design, and implementation logic.

## Architecture: The 3-View Approach

### View 1: Low-Code / Low-Effort User Configuration
*   **Target**: End-users or quick deployments.
*   **Files**: `testing.yaml`, `main-dashboard.yaml`.
*   **Responsibility**: Select a layout package and map real Home Assistant entities to that layout's expected variables.
*   **Mechanism**: Include a layout package (e.g., `!include layouts/4-tabs/package.yaml`) and use substitutions to map entities.
*   **Example**:
    ```yaml
    packages:
      hardware: !include hardware/sensecap-indicator.yaml
      layout: !include layouts/4-tabs/package.yaml
    
    substitutions:
      light_1_entity: light.living_room
      weather_entity: weather.home
      # ... etc
    ```

### View 2: Design & Widgets
*   **Target**: Designers and UI developers.
*   **Files**: `theme/`, `templates/widgets/`, `layouts/*/`.
*   **Responsibility**: Define the "look and feel" (fonts, colors, styles), the structure of individual widgets, and layout packages that combine them.
*   **Constraint**: Widgets must be "Data-Aware" (use global variables) but "Layout-Agnostic" (reusable across different layouts).
*   **Layout Packages**: Self-contained directories (e.g., `layouts/4-tabs/`) that define:
    - Tab/page structure
    - Which widgets to include
    - Default entity mappings
    - Layout-specific styling
*   **Preservation**: Existing widget templates, overlays (e.g., `light_control.yaml`), and visual cues must be reused and preserved.

### View 3: Core Implementation Layer (Logic & Data Bridge)
*   **Target**: Developers.
*   **Files**: `templates/core/`, `layouts/*/`, `templates/scripts/`.
*   **Responsibility**: Handle the "Data Bridge." Connect to Home Assistant, manage global state, and fill data buffers (e.g., for graphs).
*   **Split Architecture**:
    - **Universal Core** (`templates/core/globals.yaml`): State variables shared across ALL layouts
    - **Layout-Specific** (`layouts/*/data_bridge.yaml`, `layouts/*/widget_logic.yaml`): Entity listeners and UI refresh scripts specific to each layout
*   **Navigation Abstraction**: Manages abstract navigation actions (e.g., `ui_next`, `ui_prev`, `ui_home`) to decouple hardware inputs from UI transitions.
*   **Intentional Coupling**: Layout packages explicitly couple data bridge → globals → refresh scripts. This is by design - selecting a layout means accepting its dependencies.

## Requirements & Structure

### 1. Layout Packages
*   Layouts are self-contained packages in `layouts/` (e.g., `layouts/4-tabs/`).
*   Each layout package includes:
    - `package.yaml` - Main entry point
    - `layout.yaml` - Structure definition (tabs, grids, etc.)
    - `data_bridge.yaml` - Home Assistant entity listeners
    - `widget_logic.yaml` - UI refresh scripts
    - `globals_extension.yaml` - Layout-specific state
    - `defaults.yaml` - Fallback entity mappings
    - `id_stubs.yaml` - Stub system for optional widgets
    - `tabs/` - Individual tab definitions

### 2. Universal Components
*   **Globals** (`templates/core/globals.yaml`): State variables shared across ALL layouts (lights, scenes, weather, media, sensors).
*   **Fonts** (`templates/core/fonts.yaml`): Icon and text fonts used by all layouts.
*   **Widgets** (`templates/widgets/`): Reusable UI components (clock, weather, light tiles, etc.) that work across different layouts.
*   **Overlays** (`templates/overlays/`): Shared popup/modal components (e.g., light control overlay).
*   **Scripts** (`templates/scripts/`): Common logic (graph updates, navigation, system functions) included by layout packages.

### 3. Hardware Abstraction
*   Hardware-specific configurations (pins, display drivers) are isolated in `hardware/` templates (e.g., `sensecap-indicator.yaml`, `t-encoder_pro.yaml`).
*   Different devices can use the same layout packages with device-appropriate hardware definitions.

### 4. Data Bridge Pattern
*   **Universal Globals**: All possible state variables defined in `templates/core/globals.yaml`.
*   **Layout Listeners**: Each layout's `data_bridge.yaml` listens to HA entities it needs.
*   **Global Updates**: Data bridge updates universal globals (e.g., `light_1_state`, `weather_temp_val`).
*   **UI Refresh**: Layout's `widget_logic.yaml` reads globals and updates LVGL widgets.
*   **Trade-off**: Layouts explicitly couple to specific widget IDs. This is intentional - it makes dependencies clear.

## Development Principles

### Layout Package Philosophy
*   **Explicit Dependencies**: Each layout declares what it needs. No hidden coupling.
*   **Self-Contained**: Layout packages include everything specific to that layout (structure, data bridge, refresh scripts).
*   **Acceptable Coupling**: It's OK for `layouts/X/widget_logic.yaml` to reference specific widget IDs - that's the contract of using that layout.
*   **Null-Safety**: All refresh scripts use null checks (`if (id(foo) == nullptr) return;`) to handle optional widgets gracefully.

### Widget Design
*   **Data-Aware**: Widgets read from global variables (e.g., `id(light_1_state)`).
*   **Layout-Agnostic**: Widgets don't assume where they're placed or how they're arranged.
*   **Parameterizable**: Use `vars` to customize widget appearance (colors, sizes, positions).
*   **Reusable**: The same widget (e.g., `clock.yaml`) can be used in 4-tabs, 3-tabs, single-widget layouts.

### Hardware Abstraction
*   **Input Decoupling**: Hardware inputs (rotary, buttons, touch) should trigger abstract scripts (e.g., `ui_next`) rather than direct LVGL calls.
*   **Device Flexibility**: The same layout package can work on different hardware (SenseCap Indicator, t-encoder_pro, etc.) by swapping the hardware config.

### Creating New Layouts
1. Copy an existing layout directory (e.g., `cp -r layouts/4-tabs layouts/my-layout`)
2. Modify structure in `layout.yaml` (tabs, grids, navigation)
3. Adjust `data_bridge.yaml` for entities you need
4. Update `widget_logic.yaml` with appropriate refresh scripts
5. Set sensible defaults in `defaults.yaml`
6. Add layout-specific globals to `globals_extension.yaml` if needed

## Links
- [EspHome LVGL overview](https://esphome.io/components/lvgl/)
- [EspHome LVLG Widgets](https://esphome.io/components/lvgl/widgets/)
- [ESPHome LVGL Widtgets Tabview](https://esphome.io/components/lvgl/widgets/#tabview)
- [EspHome Tips & Tricks and Examples](https://esphome.io/cookbook/lvgl/)
- [espHome Grid Layout positioning](https://esphome.io/cookbook/lvgl/#grid-layout-positioning)
- [Sensecap Indicator EspHome sample config](https://devices.esphome.io/devices/seeed-sensecap/)
