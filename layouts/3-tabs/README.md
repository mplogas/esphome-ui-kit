# 3-Tabs Layout Package

Simplified layout package for ESPHome LVGL dashboards with 3 tabs: Home, Lights, and Scenes.

## Architecture

This layout is a **simplified version of 4-tabs** without the media player tab, making it perfect for:
- Devices where you don't need media controls
- Simpler, more focused dashboards
- Faster navigation (fewer tabs to swipe through)

## Package Contents

```
layouts/3-tabs/
├── package.yaml              # Main entry point
├── globals_extension.yaml    # Layout-specific globals (charts, UI state)
├── data_bridge.yaml          # Home Assistant entity listeners (NO media player)
├── widget_logic.yaml         # UI refresh scripts (NO media/sensor groups)
├── defaults.yaml             # Default entity mappings
├── id_stubs.yaml             # Stubs for media player (since it's not included)
├── layout.yaml               # 3-tab structure definition
└── tabs/                     # Individual tab definitions
    ├── overview.yaml         # Home tab (clock, weather, graph)
    ├── grid_4_lights.yaml    # Lights tab (4-tile grid)
    └── grid_4_scenes.yaml    # Scenes tab (4-tile grid)
```

## Usage

### In your device YAML (e.g., `testing-3tabs.yaml`):

```yaml
packages:
  hardware: !include hardware/sensecap-indicator.yaml
  layout: !include layouts/3-tabs/package.yaml  # ✨ One line!

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
  
  scene_1_entity: scene.evening
  scene_1_name: "Evening"
  scene_1_icon: "\U000F0502"
  
  # ... etc
```

## Features

### Included Functionality:
- ✅ 4 customizable lights with color/brightness control
- ✅ 4 scene activation buttons
- ✅ Weather display with rain indicator
- ✅ Power consumption graph
- ✅ Clock widget
- ✅ Light control overlay (long-press on light tiles)
- ✅ Idle timeout and backlight control

### NOT Included (vs 4-tabs):
- ❌ Media player controls
- ❌ Sensor groups
- ❌ 4th tab

### Layout-Specific Globals:
- `chart_values` - Power graph data buffer
- `temp_chart_values` - Temperature graph data buffer (optional)
- `user_is_interacting` - UI interaction state
- `current_light_entity` - Currently selected light for overlay

## Differences from 4-Tabs Layout

| Feature | 4-Tabs | 3-Tabs |
|---------|--------|--------|
| Tabs | Home, Lights, Scenes, Media | Home, Lights, Scenes |
| Media Player | ✅ | ❌ |
| Sensor Groups | ✅ | ❌ |
| Navigation Speed | Slower (4 tabs) | Faster (3 tabs) |
| Complexity | More features | Simpler, focused |

## Customization

### Optional Features:
Edit the corresponding files to enable/disable:
- **Temperature Graph**: Uncomment in `tabs/overview.yaml` and `widget_logic.yaml`
- **Additional Widgets**: Add to tab files using `!include` from `templates/widgets/`

### Stubs:
The 3-tabs layout includes stubs for media player IDs (since `templates/scripts/system_logic.yaml` references them). These stubs prevent compilation errors while keeping the layout clean.

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

### Overlays (from `templates/overlays/`):
- `light_control.yaml` - Light brightness/color control overlay

## When to Use 3-Tabs vs 4-Tabs

### Use 3-Tabs When:
- ✅ You don't need media player controls
- ✅ You want faster navigation (fewer tabs)
- ✅ You prefer simplicity over features
- ✅ You don't need sensor group displays

### Use 4-Tabs When:
- ✅ You need media player controls
- ✅ You want to display sensor data
- ✅ You need more information density
- ✅ You want the full feature set

## Migration from 4-Tabs

To migrate from 4-tabs to 3-tabs:

1. **Change the include**:
   ```yaml
   # Old:
   layout: !include layouts/4-tabs/package.yaml
   
   # New:
   layout: !include layouts/3-tabs/package.yaml
   ```

2. **Remove media substitutions** (optional - they won't be used):
   ```yaml
   # These are no longer needed:
   # media_player_entity: ...
   # media_tab_name: ...
   # sg1_s1_entity: ...
   # sg2_s1_entity: ...
   ```

3. **Compile and deploy**:
   ```bash
   esphome compile your-device.yaml
   ```

That's it! The migration is seamless.

## See Also

- [layouts/4-tabs/README.md](../4-tabs/README.md) - Full-featured 4-tabs layout
- [INSTRUCTIONS.md](../../INSTRUCTIONS.md) - Overall project architecture
- [AGENTS.md](../../AGENTS.md) - Developer guidelines
