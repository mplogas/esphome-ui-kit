# ESPHome UI Kit Project Instructions

## Project Goal
This toolkit provides a three-layer abstraction for the ESPHome LVGL component. It enables the configuration of advanced user interfaces while maintaining a strict separation between configuration, design, and implementation logic.

## Architecture: The 3-View Approach

### View 1: User Configuration
*   **Target**: End-users and deployment configurations.
*   **Files**: `testing.yaml`, `main-dashboard.yaml`.
*   **Responsibility**: Select a layout package and map Home Assistant entities to layout variables.
*   **Mechanism**: Include a layout package (e.g., `!include layouts/4-tabs/package.yaml`) and utilize substitutions for entity mapping.
*   **Example**:
    ```yaml
    packages:
      hardware: !include hardware/sensecap-indicator.yaml
      layout: !include layouts/4-tabs/package.yaml
    
    substitutions:
      light_1_entity: light.living_room
      weather_entity: weather.home
    ```

### View 2: Design and Templates
*   **Target**: UI development and styling.
*   **Files**: `theme/`, `templates/widgets/`, `layouts/*/`.
*   **Responsibility**: Define visual styles (fonts, colors, styles), specific widget structures, and composite layout packages.
*   **Guidelines**: Widgets must be data-aware (utilizing global variables) and layout-agnostic (reusable across different layout packages).
*   **Layout Packages**: Modular directories (e.g., `layouts/4-tabs/`) containing:
    - Tab and page structures
    - Widget inclusions
    - Default entity mappings
    - Layout-specific styling
*   **Preservation**: Existing widget templates, overlays (e.g., `light_control.yaml`), and visual implementations should be reused.

### View 3: Core Implementation (Logic and Data Bridge)
*   **Target**: Core component development.
*   **Files**: `templates/core/`, `layouts/*/`, `templates/scripts/`.
*   **Responsibility**: Manage the Data Bridge, Home Assistant connectivity, global state, and data buffers.
*   **Structure**:
    - **Universal Core** (`templates/core/globals.yaml`): State variables used across all layouts.
    - **Layout-Specific** (`layouts/*/data_bridge.yaml`, `layouts/*/widget_logic.yaml`): Entity listeners and UI refresh scripts specific to a single layout.
*   **Navigation Abstraction**: Implements abstract navigation actions (e.g., `ui_next`, `ui_prev`, `ui_home`) to decouple hardware inputs from UI state transitions.
*   **Coupling**: Layout packages define the specific relationships between the data bridge, global variables, and refresh scripts. This ensures explicit dependency management.

## Requirements and Structure

### 1. Layout Packages
*   Layouts are modular packages located in `layouts/` (e.g., `layouts/4-tabs/`).
*   Each layout package contains:
    - `package.yaml`: Entry point.
    - `layout.yaml`: Structure definition (tabs, grids, etc.).
    - `data_bridge.yaml`: Home Assistant entity listeners.
    - `widget_logic.yaml`: UI refresh scripts.
    - `globals_extension.yaml`: Layout-specific state.
    - `defaults.yaml`: Fallback entity mappings.
    - `id_stubs.yaml`: Stub declarations for optional widgets.
    - `tabs/`: Individual tab definitions.

### 2. Universal Components
*   **Globals** (`templates/core/globals.yaml`): State variables shared across all layouts.
*   **Fonts** (`templates/core/fonts.yaml`): Icon and text fonts.
*   **Widgets** (`templates/widgets/`): Reusable UI components compatible with multiple layouts.
*   **Overlays** (`templates/overlays/`): Shared popup and modal components.
*   **Scripts** (`templates/scripts/`): Common logic for graphs, navigation, and system functions.

### 3. Hardware Abstraction
*   Hardware-specific configurations are isolated in `hardware/` templates.
*   Devices utilize these layout packages alongside appropriate hardware definitions.

### 4. Data Bridge Pattern
*   **Universal Globals**: State variables are defined in `templates/core/globals.yaml`.
*   **Layout Listeners**: The layout's `data_bridge.yaml` monitors required Home Assistant entities.
*   **Global Updates**: The data bridge updates universal globals.
*   **UI Refresh**: `widget_logic.yaml` monitors globals and updates LVGL widgets.

## Development Principles

### Layout Package Philosophy
*   **Explicit Dependencies**: Each layout declares requirements without hidden coupling.
*   **Modularity**: Layout packages contain all layout-specific logic and structure.
*   **Intentional Coupling**: Referencing specific widget IDs within `widget_logic.yaml` is a standard part of the layout contract.
*   **Safety**: Refresh scripts implementation should verify object existence (e.g., `if (id(foo) == nullptr) return;`).

### Widget Design
*   **Data-Aware**: Widgets interface with global variables.
*   **Layout-Agnostic**: Widgets are independent of their placement or arrangement.
*   **Configurable**: Parameters passed via `vars` customize appearance and behavior.

### Hardware Abstraction
*   **Input Decoupling**: Hardware inputs trigger abstract scripts rather than direct LVGL interactions.
*   **Device Flexibility**: Layout packages are hardware-independent and compatible with various display configurations.

### Creating New Layouts
1. Clone an existing layout directory.
2. Modify the structure in `layout.yaml`.
3. Adjust `data_bridge.yaml` for required entities.
4. Update `widget_logic.yaml` with appropriate refresh scripts.
5. Set defaults in `defaults.yaml`.
6. Add layout-specific globals to `globals_extension.yaml` if needed.

## Links
- [EspHome LVGL overview](https://esphome.io/components/lvgl/)
- [EspHome LVLG Widgets](https://esphome.io/components/lvgl/widgets/)
- [ESPHome LVGL Widtgets Tabview](https://esphome.io/components/lvgl/widgets/#tabview)
- [EspHome Tips and Tricks](https://esphome.io/cookbook/lvgl/)
- [espHome Grid Layout positioning](https://esphome.io/cookbook/lvgl/#grid-layout-positioning)
- [Sensecap Indicator EspHome configuration](https://devices.esphome.io/devices/seeed-sensecap/)
