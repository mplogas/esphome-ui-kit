# 4-Tabs Layout Package

Modular layout package for ESPHome LVGL dashboards. Includes four tabs: Home, Lights, Scenes, and Sensors.

## Architecture

This layout implements the **3-View Architecture** pattern:
- **View 1 (User Configuration)**: Includes this package and maps Home Assistant entities.
- **View 2 (Design)**: Utilizes reusable widgets from `templates/widgets/`.
- **View 3 (Core Logic)**: Manages the data bridge (entity listeners), widget logic (refresh scripts), and state management.

## Package Contents

```
layouts/4-tabs/
├── package.yaml              # Entry point for configuration inclusion
├── globals_extension.yaml    # Layout-specific global variables
├── data_bridge.yaml          # Home Assistant entity listeners
├── widget_logic.yaml         # UI refresh scripts
├── defaults.yaml             # Default entity substitutions
├── id_stubs.yaml             # ID stub declarations for optional components
├── layout.yaml               # Tabview structural definition
└── tabs/                     # Tab-specific definitions
    ├── overview.yaml         # Home tab (clock, weather, media player)
    ├── grid_4_lights.yaml    # Lights tab (4-tile grid)
    ├── grid_4_scenes.yaml    # Scenes tab (4-tile grid)
    └── sensors.yaml          # Sensors tab (dual graphs and sensor groups)
```

## Usage

### Configuration Example (`testing.yaml`):

```yaml
packages:
  hardware: !include hardware/sensecap-indicator.yaml
  layout: !include layouts/4-tabs/package.yaml

substitutions:
  # Load theme defaults
  <<: !include theme/defaults.yaml
  
  # Device identification
  name: my-device
  friendly_name: My Device
  
  # Map Home Assistant entities
  weather_entity: weather.home
  graph_1_entity: sensor.home_power
  graph_2_entity: sensor.home_temp
  
  light_1_entity: light.living_room
  light_1_name: "Living Room"
  light_1_icon: "\U000F0335"
```

## Features

### Functional Components:
- Four customizable light tiles with color and brightness control.
- Four scene activation buttons.
- Weather display with indicators.
- Dual sensor graphs (e.g., power and temperature).
- Clock and media player widgets.
- Dual sensor groups (supporting up to six sensors).
- Light control overlay (accessible via long-press).
- System features: idle timeout and backlight management.

### Layout-Specific Globals:
- `graph_1_values`: Primary graph data buffer.
- `graph_2_values`: Secondary graph data buffer.
- `user_is_interacting`: UI interaction state monitoring.
- `current_light_entity`: Selected light entity for the control overlay.

## Customization

### Optional Components:
Modify the following components as required:
- **Sensor Tab**: The fourth tab includes `graph_1`, `graph_2`, `sg1`, and `sg2`.
- **Widget Inclusion**: Additional widgets can be added from `templates/widgets/`.

### ID Stubs:
When removing optional widgets while retaining their logic, enable the corresponding stubs in `id_stubs.yaml` to ensure compilation success.

## Dependencies

### Universal Components (`templates/core/`):
- `globals.yaml`: Universal state variables.
- `fonts.yaml`: Icon and typographic definitions.
- `grid_container.yaml`: Grid layout implementation.
- `tabview_config.yaml`: Tab navigation settings.

### Scripts (`templates/scripts/`):
- `graph_logic.yaml`: Chart initialization and data management.
- `system_logic.yaml`: Clock, navigation, and power logic.
- `lights_4.yaml`: Light overlay and color management.

### Widgets (from `templates/widgets/`):
All widgets are reusable across layouts:
- `clock.yaml`, `weather.yaml`, `graph.yaml`
- `light_tile.yaml`, `scene_tile.yaml`
- `media_player.yaml`, `sensor_group.yaml`

### Overlays (from `templates/overlays/`):
- `light_control.yaml` - Light brightness/color control overlay

## Creating New Layouts

To create a new layout (e.g., 3-tabs, single-widget, 2x3-grid):

1. **Copy this directory**: `cp -r layouts/4-tabs layouts/my-new-layout`
2. **Modify structure**: Edit `layout.yaml` to define your tab/widget structure
3. **Update data bridge**: Edit `data_bridge.yaml` to listen to entities you need
4. **Adjust refresh scripts**: Edit `widget_logic.yaml` for your widgets
5. **Update defaults**: Edit `defaults.yaml` with sensible fallbacks
6. **Update globals**: Edit `globals_extension.yaml` if you need layout-specific state

## See Also

- [INSTRUCTIONS.md](../../INSTRUCTIONS.md) - Overall project architecture
- [AGENTS.md](../../AGENTS.md) - Developer guidelines and constraints
- [theme/](../../theme/) - Color schemes and styles
