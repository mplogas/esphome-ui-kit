# ESPHome UI Kit - Project Instructions

## Goal
A lightweight, 3-layer abstraction over ESPHome's LVGL feature. The goal is to quickly set up sophisticated UIs for various ESPHome devices with displays (e.g., SenseCap Indicator) while maintaining a strict separation between configuration, design, and implementation logic.

## Architecture: The 3-View Approach

### View 1: Low-Code / Low-Effort User Configuration)
*   **Target**: End-users or quick deployments.
*   **Files**: `testing.yaml`, `main-dashboard.yaml`, `mapping-defaults.yaml`.
*   **Responsibility**: Define which widgets go into which slots and map real Home Assistant entities to internal data slots.
*   **Mechanism**: Uses substitutions to assign widget files to grid slots (e.g., `tab_1_slot_1_1_widget: "templates/tiles/light_tile.yaml"`).

### View 2: CI (Design & Widgets) Configuration
*   **Target**: Designers and UI developers.
*   **Files**: `theme/`, `templates/tiles/`, `templates/layouts/`.
*   **Responsibility**: Define the "look and feel" (fonts, colors, styles) and the structure of individual widgets.
*   **Constraint**: Widgets must be "Data-Aware" but "Layout-Agnostic." They should update themselves based on global variables and should not rely on hardcoded container IDs from other layers.
*   **Preservation**: Existing widget templates, overlays (e.g., `light_control.yaml`), and visual cues must be reused and preserved.

### View 3: Core Implementation Layer (Logic & Data Bridge)
*   **Target**: Developers.
*   **Files**: `templates/core/`, `templates/scripts/`.
*   **Responsibility**: Handle the "Data Bridge." Connect to Home Assistant, manage global state, and fill data buffers (e.g., for graphs).
*   **Navigation Abstraction**: Manages abstract navigation actions (e.g., `ui_next`, `ui_prev`, `ui_home`, `ui_action`) to decouple hardware inputs (rotary, buttons, touch) from UI transitions.
*   **Constraint**: This layer must **never** have hard links to UI container IDs. It updates global variables; the UI (Layer 2) chooses whether or not to display that data.

## Requirements & Structure

### 1. Root Layout & Navigation
*   A `tabview` is the root layout, providing swiping and navigation.
*   Supports up to 4 tabs.
*   Tab properties (position, style, names) are configurable via substitutions.

### 2. Grid System (The Slots)
*   Each tab contains a 2x2 grid container.
*   **Slot Naming**: `tab_[1-4]_slot_[1-4]_widget` (e.g., `tab_1_slot_1_1` for Row 1, Col 1).
*   Slots are fully flexible; any widget can be placed in any slot.
*   Empty slots should use a `blank_tile.yaml` to maintain layout integrity.

### 3. Data Pool & Mapping
*   **Standard Data Slots**:
    *   4 Lights (State, Brightness, Color)
    *   4 Scenes
    *   1 Media Player (Title, State, Position, Duration)
    *   2 Sensor Groups (3 sensors each)
    *   1 Weather + Rain Indicator
    *   1 Clock
*   **Mapping**: A `mapping-defaults.yaml` provides fallback values for all entities to ensure the project always compiles.

### 4. Hardware Abstraction
*   Hardware-specific configurations (pins, display drivers) are isolated in `hardware/` templates (e.g., `sensecap-indicator.yaml`).

## Development Principles
*   **Decoupling**: If a UI component is removed, the compilation must not fail. Use global variables as the "bridge" between sensors and UI.
*   **Input Decoupling**: Hardware inputs (rotary, buttons, touch) should trigger abstract scripts (e.g., `ui_next`) rather than direct LVGL widget calls. This allows the same logic to work across different device types.
*   **Self-Containment**: Widgets should "own" their logic, including touch events and internal refresh timers.
*   **Reuse & Adaptability**: Do not reinvent existing UI elements. Adapt current templates to the 3-view model. While logic is shared via View 3, View 2 may provide device-specific widget variants (e.g., round vs. square) to accommodate different screen geometries.

## Links
- [EspHome LVGL overview](https://esphome.io/components/lvgl/)
- [EspHome LVLG Widgets](https://esphome.io/components/lvgl/widgets/)
- [ESPHome LVGL Widtgets Tabview](https://esphome.io/components/lvgl/widgets/#tabview)
- [EspHome Tips & Tricks and Examples](https://esphome.io/cookbook/lvgl/)
- [espHome Grid Layout positioning](https://esphome.io/cookbook/lvgl/#grid-layout-positioning)
- [Sensecap Indicator EspHome sample config](https://devices.esphome.io/devices/seeed-sensecap/)
