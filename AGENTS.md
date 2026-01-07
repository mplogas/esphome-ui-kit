# Agent Instructions: ESPHome UI Kit

> **📖 First Time Here?** Read [INSTRUCTIONS.md](INSTRUCTIONS.md) for the architecture philosophy and design vision. This file focuses on practical ESPHome constraints, compilation workflows, and troubleshooting.

## Quick Reference
- **Architecture Vision**: See [INSTRUCTIONS.md - The 3-View Approach](INSTRUCTIONS.md#architecture-the-3-view-approach)
- **Project Goals**: See [INSTRUCTIONS.md - Goal](INSTRUCTIONS.md#goal)
- **This File**: ESPHome constraints, compilation, debugging, security

---

## CRITICAL: Development Environment
Before running any ESPHome commands, you **MUST** activate the virtual environment:
```bash
. ~/esphome/.venv/bin/activate
```
**Terminal Tip**: If you get a "zsh: correct..." prompt, disable it with `unsetopt correct` or run `python3 -m esphome`.

To verify changes, always run:
```bash
esphome compile testing.yaml
```
*Note: `testing.yaml` is the primary development file. Only once a feature is stable should you update `main-dashboard.yaml`.*

### Rule D: Configuration Preservation
**NEVER** suggest or write a "from-scratch" root configuration file. 
*   Always reuse the existing `testing.yaml`.
*   Maintain the user's `substitutions`, `wifi`, `api`, `ota`, and `time` blocks. 
*   If you need to add a package or a global, add it to the existing `packages:` or `substitutions:` block rather than replacing the whole file.

### Rule E: Terminal Context Priority

When executing terminal commands or reading terminal state:

**ALWAYS check the Terminal context provider FIRST** before using `run_terminal_command`.

**Why This Matters:**
- The `run_terminal_command` tool may run in a different shell session/context than the user's active VSCode terminal
- This can lead to path mismatches (e.g., `/home/mplogas` vs `/home/marc/esphome/config/esphome-ui-kit`)
- The user's terminal shows the ACTUAL working directory and git state

**Best Practices:**
1. ✅ **Reference Terminal context** if user shows recent output (they can paste it)
2. ✅ **Ask for Terminal output** before making assumptions about paths/state
3. ✅ **Verify location** before git operations (check `pwd` and `git status` from Terminal context)
4. ⚠️ **Use `run_terminal_command` cautiously** - it may not match user's active terminal
5. ❌ **Don't assume** you're in the correct directory without verification

**Example Workflow:**
```
User: "Create git commits for my changes"
Agent: "I can see you're in /home/marc/esphome/config/esphome-ui-kit 
        on branch refactor/3-view-architecture with 5 modified files.
        Let me create logical commits..."
```

---

## 0. ESPHome Platform Constraints

**This is NOT a typical software project.** ESPHome is firmware configuration, not application code.

### What ESPHome IS:
- ✅ YAML-based firmware configuration
- ✅ Limited C++ lambdas for custom logic
- ✅ Script system with basic parameters
- ✅ ~4MB flash, 320KB RAM (ESP32-S3)
- ✅ Compiled once, runs forever
- ✅ Can include custom C++ headers

### What ESPHome is NOT:
- ❌ Full C++ application with OOP patterns
- ❌ Supports dependency injection
- ❌ Has event bus or pub/sub libraries
- ❌ Runtime reflection or dynamic object querying
- ❌ Can interpolate IDs in lambdas (no `id(${variable}_icon)`)
- ❌ `globals:` supports std::deque or custom containers (only `std::vector`)

### Critical Implications:

**1. ID References Must Be Known at Compile Time**
```cpp
// ❌ This will NOT compile (even with null check):
if (id(light_1_icon) == nullptr) return;  // Error: undeclared identifier

// ✅ ID must exist somewhere (layout or stub) for compilation to succeed
```

**2. Cannot Parameterize ID References**
```yaml
# ❌ This is IMPOSSIBLE in ESPHome:
script:
  - id: refresh_light
    parameters:
      light_id: std::string
    then:
      - lambda: |-
          id(${light_id}_icon).set_color(...)  # Can't interpolate
```

**3. Duplication is Sometimes Unavoidable**
- We have 4 separate `light_X_refresh` scripts instead of one parameterized script
- This is an accepted compromise given platform constraints

**4. Stubs are Compile-Time, Not Runtime**
- Stubs satisfy ESPHome's compiler, not fix runtime null pointers
- Null checks handle runtime safety, stubs handle compilation

### Why This Matters for the 3-View Architecture:

The **ideal** "layout-agnostic" principle from [INSTRUCTIONS.md](INSTRUCTIONS.md#view-3-core-implementation-layer) is not always achievable. View 3 scripts sometimes reference View 2 IDs directly (see `templates/core/widget_logic.yaml` header comments).

**This is an acceptable compromise**, not a design flaw.

---

## 1. The 3-View Architecture (Strict Separation)

> **Philosophy**: See [INSTRUCTIONS.md - Architecture](INSTRUCTIONS.md#architecture-the-3-view-approach) for the vision. This section covers practical implementation.

### View 1: User Configuration (`testing.yaml`, `templates/core/mapping-defaults.yaml`)
*   **Role**: Assigns widgets to slots using substitutions.
*   **Rule**: Never add logic here. Only definitions.
*   **Pattern**: Uses `tab_[N]_slot_[X]_[Y]_widget` to point to a file in `templates/`.
*   **Files**: `testing.yaml`, `templates/core/mapping-defaults.yaml`

### View 2: Design & Widgets (`templates/widgets/`, `templates/widgets/`, `theme/`)
*   **Role**: Visual representation.
*   **Rule**: Must be **Data-Aware** (use global variables) but **Layout-Agnostic** (don't assume where they are placed).
*   **Implementation**: Use `vars` passed from View 1 to set internal IDs and entities.
*   **Naming Convention**: Use `${id}` passed via vars for the root object ID of a widget.
*   **Files**: `templates/widgets/*.yaml`, `templates/widgets/*.yaml`, `theme/*.yaml`

### View 3: Core Logic (`templates/core/`, `templates/scripts/`)
*   **Role**: The "Data Bridge". Fetches data from Home Assistant and updates Global Variables.
*   **Rule**: **NEVER** reference an LVGL object ID directly unless you use a check or a `stubs` system.
*   **Files**: `templates/core/data_bridge.yaml`, `templates/core/globals.yaml`, `templates/core/widget_logic.yaml`
*   **Reality Check**: Some View 3 scripts DO reference View 2 IDs directly. This is documented in the code with explanations.
*   **The Bridge Pattern**: 
    1. HA Sensor -> `data_bridge.yaml` 
    2. `data_bridge.yaml` -> Updates `globals.yaml` 
    3. `globals.yaml` -> Widget (View 2) reads this to update itself

---

## 2. Preventing "Fix-Error-Loop" & Crashes

### Rule A: The "Stub" Requirement (Understanding Compile-Time vs Runtime)

**The Problem:**
ESPHome validates ALL `id()` references during compilation, even inside null checks.

**Example:**
```cpp
// This WILL NOT COMPILE if light_1_icon doesn't exist:
if (id(light_1_icon) == nullptr) return;  // ❌ Compile error: undeclared identifier
```

**The Solution:**
Stubs declare IDs in a hidden page (`templates/core/id_stubs.yaml`) so ESPHome's compiler sees them.

**When You Need Stubs:**
1. ✅ A script references an ID that's not in your active layout
2. ✅ You get "undeclared identifier" compile errors
3. ❌ Runtime crashes (null checks handle this)

**Trade-off:**
- Stubs add ~10 bytes per object to your binary
- But they enable flexible layouts without breaking scripts
- Most stubs are commented out by default (only uncomment when needed)

### Rule B: Global Variable Fallbacks
Always initialize global variables with safe defaults in `globals.yaml`. Use `nan` for floats or empty strings for text to avoid rendering garbage before HA connects.

### Rule C: Decoupling Check
If deleting a widget breaks the build, it means you have a **"Hard Link"** in your logic. 
*   **Fix**: Move the logic that references that widget's ID into a script, or use a lambda check if possible.

---

## 3. Workflow for New Features

**Quick Reference** - Follow these steps in order:

1.  **Define Globals**: Add necessary variables to `templates/core/globals.yaml`.
2.  **Implement Data Bridge**: Add the HA sensor/binary_sensor to `templates/core/data_bridge.yaml`. Update the global variable on state change.
3.  **Create Refresh Script**: Add script to `templates/core/widget_logic.yaml` to update UI from globals.
4.  **Create/Modify Widget**: Update `templates/widgets/`. Use the global variables for values.
5.  **Update Stubs (Optional)**: Add any mandatory IDs to `templates/core/id_stubs.yaml` if needed.
6.  **Configure in YAML**: Map the widget in `testing.yaml` (or `templates/core/mapping-defaults.yaml` for defaults).
7.  **Compile & Validate**: Run `esphome compile testing.yaml`.
8.  **Test**: Upload to device and verify functionality.

**See Section 8** for a complete step-by-step example.

## 4. Troubleshooting

### Compilation Errors

#### "Undeclared identifier 'light_1_icon'"
**Cause**: A script in `widget_logic.yaml` references an ID not in your active layout.

**Fix**: Uncomment the corresponding stub in `templates/core/id_stubs.yaml`:
```yaml
# Uncomment this:
- label: { id: light_1_icon }
```

**Why it happens**: ESPHome validates IDs at compile time, even inside `if (id(foo) == nullptr)` checks.

---

#### "Component already defined" or "ID already defined"
**Cause**: You included a template twice OR a stub conflicts with a real object.

**Fix**: 
1. Search for duplicate `id: foo` declarations across all YAML files
2. Check if you have both a stub AND a real widget with the same ID
3. Remove the duplicate or comment out the stub

---

#### "Component not found"
**Cause**: Missing `!include` in the root YAML or wrong file path.

**Fix**:
1. Check `testing.yaml` has all required includes in `packages:`
2. Verify file paths are correct (relative to root)
3. Check for typos in filenames

---

#### Indentation Errors
**Cause**: ESPHome's multi-level includes with `vars` are extremely sensitive to spacing.

**Fix**: 
- Use a YAML validator
- Ensure consistent indentation (2 spaces recommended)
- Check that `vars:` blocks are properly indented

---

### Substitution Cascading Issues

**Problem**: ESPHome `!include` with `vars` does NOT automatically pass variables down multiple levels.

**Example**:
```yaml
# testing.yaml
- !include
    file: layout.yaml
    vars:
      my_var: "value"

# layout.yaml
- !include
    file: widget.yaml
    # ❌ widget.yaml CANNOT see my_var
```

**Fix**: Explicitly pass variables at each level:
```yaml
# layout.yaml
- !include
    file: widget.yaml
    vars:
      my_var: "${my_var}"  # ✅ Pass it through
```

---

### Runtime Issues

#### RGB Colors Show Garbage on Boot
**Cause**: Global variables initialize to 0 before Home Assistant connects.

**Fix**: This is expected behavior. `data_bridge.yaml` updates them once HA sends data. Consider adding default colors in `globals.yaml`:
```yaml
- id: light_1_color_val
  type: uint32_t
  initial_value: '0xFFFFFF'  # Default to white
```

---

#### Graph Not Updating
**Cause**: Chart container object is nullptr or chart initialization failed.

**Fix**:
1. Check ESPHome logs for "Chart user_data is null" warnings
2. Verify `init_graph` script ran in `esphome.on_boot`
3. Check that `home_power_graph_chart_container` exists in your layout

---

#### Widget Appears Frozen
**Cause**: Refresh script isn't being called, or Home Assistant isn't sending updates.

**Debug**:
1. Add logging to refresh scripts:
   ```yaml
   - lambda: |-
       ESP_LOGD("refresh", "light_1_refresh called");
   ```
2. Check Home Assistant logs for entity updates
3. Verify entity IDs are correct in `testing.yaml`

---

#### Removing a Widget Breaks Compilation
**Cause**: A script in View 3 still references the widget's IDs.

**Fix** (choose one):
1. Keep the script (null checks prevent crashes)
2. Comment out the script in `widget_logic.yaml`
3. Add stubs for the IDs in `id_stubs.yaml`

**See**: `templates/core/widget_logic.yaml` header comments for details.

---

## 5. Security & Input Validation

### Always Validate Home Assistant Data

Home Assistant can send unexpected values. Always validate and sanitize.

#### Clamp Numeric Values
```yaml
# ✅ Good:
on_value:
  - lambda: |-
      int val = x;
      val = val < 0 ? 0 : (val > 255 ? 255 : val);  # Clamp to 0-255
      id(my_global) = val;

# ❌ Bad:
on_value:
  - lambda: 'id(my_global) = x;'  # Could be negative or > 255
```

#### Log Parsing Failures
```yaml
on_value:
  - lambda: |-
      int r, g, b;
      if (sscanf(x.c_str(), "[%d,%d,%d]", &r, &g, &b) == 3) {
        // Clamp and use values
      } else {
        ESP_LOGW("data_bridge", "Failed to parse: %s", x.c_str());
      }
```

#### Null-Check Before Accessing LVGL Objects
```yaml
- lambda: |-
    if (id(my_label) == nullptr) return;
    lv_label_set_text(id(my_label), "text");
```

#### Validate Before Bit Operations
**Common Cases:**
- RGB colors: Must be 0-255 per channel
- Brightness: Must be 0-255
- Percentages: Must be 0-100
- Floats: Check for NaN with `std::isnan(x)`

**Example** (from `data_bridge.yaml`):
```yaml
text_sensor:
  - platform: homeassistant
    attribute: rgb_color
    on_value:
      - lambda: |-
          int r, g, b;
          if (sscanf(x.c_str(), "[%d,%d,%d]", &r, &g, &b) == 3) {
            // ✅ Clamp to valid range
            r = r < 0 ? 0 : (r > 255 ? 255 : r);
            g = g < 0 ? 0 : (g > 255 ? 255 : g);
            b = b < 0 ? 0 : (b > 255 ? 255 : b);
            id(light_color_val) = (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
          } else {
            ESP_LOGW("rgb", "Bad format: %s", x.c_str());
          }
```

---

## 6. C++ Header Best Practices

### Location & Purpose
Custom C++ headers live in `src/*.h` and provide functionality beyond ESPHome's lambdas.

**Current Example**: `src/lvgl_chart.h` provides chart initialization and smoothing.

### Guidelines

1. **Always Validate Pointer Parameters**
```cpp
void update_widget(lv_obj_t* obj, const std::vector<float>* data) {
    // ✅ Validate everything
    if (obj == nullptr || data == nullptr || data->empty()) {
        ESP_LOGW("widget", "Invalid parameters");
        return;
    }
```

2. **Pre-allocate Containers**
```cpp
std::vector<int32_t> processed;
processed.reserve(data->size());  // ✅ Prevent repeated reallocations
```

3. **Use Static Casts, Not C-Style**
```cpp
// ✅ Good:
auto* series = static_cast<lv_chart_series_t*>(chart->user_data);

// ❌ Avoid:
lv_chart_series_t* series = (lv_chart_series_t*)chart->user_data;
```

4. **Validate user_data Before Casting**
```cpp
if (obj->user_data == nullptr) {
    ESP_LOGW("tag", "user_data is null");
    return;
}
auto* data = static_cast<MyType*>(obj->user_data);
```

5. **Keep Functions Pure (No Side Effects)**
- Don't modify global state
- Return values or use output parameters
- Makes code testable and predictable

6. **Use ESP Logging**
```cpp
ESP_LOGD("tag", "Debug message: %d", value);  // Debug (verbose)
ESP_LOGI("tag", "Info message");              // Info
ESP_LOGW("tag", "Warning: %s", msg);          // Warning
ESP_LOGE("tag", "Error!");                    // Error
```

### Memory Safety Example
```cpp
void update_lvgl_chart(lv_obj_t* chart, const std::vector<float>* values, float min_val, float max_val) {
    // ✅ Validate everything first
    if (chart == nullptr || values == nullptr || values->empty()) return;
    
    if (chart->user_data == nullptr) {
        ESP_LOGW("lvgl_chart", "Chart user_data is null, cannot update");
        return;
    }
    
    // ✅ Safe cast
    auto* ser = static_cast<lv_chart_series_t*>(chart->user_data);
    
    // ✅ Pre-allocate
    std::vector<int32_t> smoothed_points;
    smoothed_points.reserve(values->size());
    
    // ... rest of logic
}
```

---

## 7. Performance Considerations

### Memory Usage

**Chart Buffers:**
- Default: 60 points × 4 bytes per float × 2 charts = **480 bytes**
- Acceptable on ESP32-S3 (320KB RAM)
- Configure via `${chart_max_points}` substitution

**Vector Operations:**
- `vector.erase(begin())` is O(n) but fine for <100 points
- Don't use `std::deque` - not supported in ESPHome globals
- Use `reserve()` when final size is known

### Update Frequency

**Low Frequency** (state changes only):
- Light refresh: When HA sends state change
- Scene buttons: On press
- Weather: Every few minutes (HA-controlled)

**Medium Frequency** (sensor polling):
- Chart updates: Configure `update_interval` in sensor definition
- Recommended: 5-60 seconds for power/temp sensors

**High Frequency** (UI updates):
- Clock: Every 1 second (see `system_logic.yaml`)
- Media seekbar: Every 1 second if playing

**Optimization Tip**: Don't update widgets when display is off:
```cpp
if (!id(backlight).remote_values.is_on()) return;
```

---

## 8. Complete Example: Adding a New Widget

Let's add a humidity sensor widget from scratch.

### Step 1: Define Global Variable
```yaml
# templates/core/globals.yaml
globals:
  - id: my_humidity_val
    type: float
    initial_value: '0'
```

### Step 2: Create Data Bridge
```yaml
# templates/core/data_bridge.yaml
sensor:
  - platform: homeassistant
    id: internal_my_humidity
    entity_id: sensor.living_room_humidity
    on_value:
      - lambda: 'id(my_humidity_val) = x;'
      - script.execute: my_humidity_refresh
```

### Step 3: Create Refresh Script
```yaml
# templates/core/widget_logic.yaml
script:
  - id: my_humidity_refresh
    then:
      - lambda: |-
          if (id(my_humidity_label) == nullptr) return;
          char buf[16];
          sprintf(buf, "%.0f%%", id(my_humidity_val));
          lv_label_set_text(id(my_humidity_label), buf);
```

### Step 4: Create Widget Template
```yaml
# templates/widgets/my_humidity.yaml
obj:
  id: ${widget_id}
  width: 100%
  height: 100%
  styles: style_card
  grid_cell_column_pos: ${grid_col_pos}
  grid_cell_row_pos: ${grid_row_pos}
  widgets:
    - label:
        text: "\U000F058E"  # mdi:water-percent
        text_font: icons35
        align: CENTER
        y: -20
        text_color: ${color_primary}
    - label:
        id: my_humidity_label
        text: "--"
        text_font: ${font_id_large}
        align: CENTER
        y: 20
        text_color: ${color_text}
```

### Step 5: Add to Layout
```yaml
# templates/tabs/overview.yaml (or your custom tab)
widgets:
  - <<: !include
      file: ../widgets/my_humidity.yaml
      vars:
        widget_id: ui_humidity
        grid_col_pos: 0
        grid_row_pos: 1
```

### Step 6: Configure Substitution
```yaml
# testing.yaml
substitutions:
  my_humidity_entity: sensor.living_room_humidity
```

### Step 7: Add Stub (if needed)
```yaml
# templates/core/id_stubs.yaml
# Uncomment if you later remove the widget but keep the script:
#- label: { id: my_humidity_label }
```

### Step 8: Compile & Test
```bash
. ~/esphome/.venv/bin/activate
esphome compile testing.yaml
```

**Check logs**:
- Look for "my_humidity_refresh called" if you added logging
- Verify Home Assistant is sending updates
- Check for any nullptr warnings

---

## 9. Style & UI Guidelines
*   Use `${grid_gap}` and `${grid_padding}` for alignment.
*   Reuse styles from `theme/style.yaml` via `style_definitions`.
*   Keep icons consistent using MDI hex codes (e.g., `"\U000F0335"`).
*   Follow the color scheme defined in `theme/defaults.yaml`.
*   Use consistent font sizes: `${font_id_small}`, `${font_id}`, `${font_id_large}`.
