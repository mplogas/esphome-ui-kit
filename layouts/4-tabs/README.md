# 4-Tabs Layout Package

Self-contained layout package for ESPHome LVGL dashboards with 4 tabs: Home, Lights, Scenes, and Media.

## Architecture

This layout follows the **3-View Architecture** pattern:
- **View 1 (User Config)**: User just includes this package and maps entities
- **View 2 (Design)**: Reusable widgets from `templates/widgets/`
- **View 3 (Core Logic)**: Data bridge, refresh scripts, and state management

## Package Contents

```
layouts/4-tabs/
├── package.yaml              # Main entry point - include this in your config
├── globals_extension.yaml    # Layout-specific globals (charts, UI state)
├── data_bridge.yaml          # Home Assistant entity listeners
├── widget_logic.yaml         # UI refresh scripts
├── defaults.yaml             # Default entity mappings (fallbacks)
├── id_stubs.yaml             # Stub declarations for optional widgets
├── layout.yaml               # Tabview structure definition
└── tabs/                     # Individual tab definitions
    ├── overview.yaml         # Home tab (clock, weather, graph, sensors)
    ├── grid_4_lights.yaml    # Lights tab (4-tile grid)
    ├── grid_4_scenes.yaml    # Scenes tab (4-tile grid)
    └── media_sensors.yaml    # Media tab (media player + sensors)
```

## Usage

### In your device YAML (e.g., `testing.yaml`):

```yaml
packages:
  hardware: !include hardware/sensecap-indicator.yaml
  layout: !include layouts/4-tabs/package.yaml  # ✨ One line!

substitutions:
  # Load theme
  <<: !include theme/defaults.yaml
  
  # Device identification
  name: my-device
  friendly_name: My Device
  
  # Map your Home Assistant entities
  weather_entity: weather.home
  power_sensor_entity: sensor.home_power
  
  light_1_entity: light.living_room
  light_1_name: "Living Room"
  light_1_icon: "\U000F0335"
  
  # ... etc
```

## Features

### Included Functionality:
- ✅ 4 customizable lights with color/brightness control
- ✅ 4 scene activation buttons
- ✅ Weather display with rain indicator
- ✅ Power consumption graph
- ✅ Clock widget
- ✅ Media player controls
- ✅ Sensor groups (temperature, humidity, CO2)
- ✅ Light control overlay (long-press on light tiles)
- ✅ Idle timeout and backlight control

### Layout-Specific Globals:
- `chart_values` - Power graph data buffer
- `temp_chart_values` - Temperature graph data buffer (optional)
- `user_is_interacting` - UI interaction state
- `current_light_entity` - Currently selected light for overlay

## Customization

### Optional Features:
Edit the corresponding files to enable/disable:
- **Temperature Graph**: Uncomment in `tabs/overview.yaml` and `widget_logic.yaml`
- **Sensor Group 2**: Uncomment in `tabs/media_sensors.yaml` and `data_bridge.yaml`
- **Additional Widgets**: Add to tab files using `!include` from `templates/widgets/`

### Stubs:
If you remove optional widgets but keep their scripts, uncomment the corresponding stubs in `id_stubs.yaml` to prevent compilation errors.

## Dependencies

### Universal (from `templates/core/`):
- `globals.yaml` - Universal state variables (lights, scenes, weather, etc.)
- `fonts.yaml` - Icon and text fonts
- `grid_container.yaml` - Grid layout helper
- `tabview_config.yaml` - Tab navigation configuration

### Scripts (from `templates/scripts/`):
- `graph_logic.yaml` - Chart initialization and updates
- `system_logic.yaml` - Clock, navigation, wake-up logic
- `lights_4.yaml` - Light overlay and color picker logic

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
