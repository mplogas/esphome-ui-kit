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
**ALWAYS check Terminal context FIRST** before using `run_terminal_command` (it may run in a different shell/directory). Ask user to paste terminal output if unclear.

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

### Why This Matters for Layout Packages:

Layout packages (**View 2 + View 3 combined**) intentionally couple refresh scripts to specific widget IDs. This is not a design flaw - it's an explicit contract:

**"If you use the 4-tabs layout, you get these scripts and these dependencies."**

The trade-off: Layouts are self-contained and explicit about what they need, but removing a widget from a layout requires either keeping the script (null checks prevent crashes) or adding stubs.

---

## 1. The 3-View Architecture (Strict Separation)

> **Philosophy**: See [INSTRUCTIONS.md - Architecture](INSTRUCTIONS.md#architecture-the-3-view-approach) for the vision. This section covers practical implementation.

### View 1: User Configuration (`testing.yaml`, `main-dashboard.yaml`)
*   **Role**: Select a layout package and map Home Assistant entities.
*   **Rule**: Never add logic here. Only definitions and entity mappings.
*   **Pattern**: Include a layout package (e.g., `!include layouts/4-tabs/package.yaml`) and define entity substitutions.
*   **Files**: `testing.yaml`, `main-dashboard.yaml`

### View 2: Design & Widgets (`templates/widgets/`, `layouts/`, `theme/`)
*   **Role**: Visual representation and layout structure.
*   **Rule**: Widgets must be **Data-Aware** (use global variables) but **Layout-Agnostic** (don't assume where they are placed).
*   **Implementation**: Use `vars` passed from layouts to set internal IDs and entities.
*   **Naming Convention**: Use `${widget_id}` passed via vars for the root object ID of a widget.
*   **Files**: `templates/widgets/*.yaml`, `layouts/*/`, `theme/*.yaml`

### View 3: Core Logic (`templates/core/`, `layouts/*/`, `templates/scripts/`)
*   **Role**: The "Data Bridge". Fetches data from Home Assistant and updates Global Variables.
*   **Split Responsibility**: 
    - **Universal Core** (`templates/core/globals.yaml`): State variables shared across ALL layouts
    - **Layout-Specific** (`layouts/*/data_bridge.yaml`, `layouts/*/widget_logic.yaml`): Entity listeners and refresh scripts specific to a layout
*   **Rule**: Layout packages explicitly declare their dependencies. Scripts reference View 2 IDs directly (with null checks for safety).
*   **Reality**: Tight coupling between a layout and its scripts is **intentional** - choosing a layout means accepting its dependencies.
*   **The Bridge Pattern**: 
    1. HA Sensor -> `layouts/X/data_bridge.yaml` 
    2. `data_bridge.yaml` -> Updates `templates/core/globals.yaml` 
    3. `globals.yaml` -> Widget (View 2) reads this to update itself
    4. `layouts/X/widget_logic.yaml` -> Refreshes UI from globals

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
Stubs declare IDs in a hidden page (`layouts/X/id_stubs.yaml`) so ESPHome's compiler sees them.

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

1.  **Define Globals**: Add necessary variables to `templates/core/globals.yaml` (universal) or `layouts/X/globals_extension.yaml` (layout-specific).
2.  **Implement Data Bridge**: Add the HA sensor/binary_sensor to `layouts/X/data_bridge.yaml`. Update the global variable on state change.
3.  **Create Refresh Script**: Add script to `layouts/X/widget_logic.yaml` to update UI from globals.
4.  **Create/Modify Widget**: Update `templates/widgets/`. Use the global variables for values.
5.  **Update Stubs (Optional)**: Add any mandatory IDs to `layouts/X/id_stubs.yaml` if needed.
6.  **Configure in YAML**: Map entities in `testing.yaml` substitutions.
7.  **Compile & Validate**: Run `esphome compile testing.yaml`.
8.  **Test**: Upload to device and verify functionality.

**See Section 8** for a complete step-by-step example.

## 4. Troubleshooting

### Compilation Errors

#### "Undeclared identifier 'light_1_icon'"
**Cause**: A script in `layouts/X/widget_logic.yaml` references an ID not in your active layout.

**Fix**: Uncomment the corresponding stub in `layouts/X/id_stubs.yaml`:
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
**Cause**: A script in the layout package still references the widget's IDs.

**Fix** (choose one):
1. Keep the script (null checks prevent crashes)
2. Comment out the script in `layouts/X/widget_logic.yaml`
3. Add stubs for the IDs in `layouts/X/id_stubs.yaml`

**Note**: Layouts explicitly declare their dependencies. This coupling is intentional.

---

## 5. Security & Input Validation

**Always validate Home Assistant data** - it can send unexpected values.

**Key Rules:**
- **Clamp numeric values**: RGB (0-255), brightness (0-255), percentages (0-100)
- **Null-check LVGL objects**: `if (id(my_label) == nullptr) return;`
- **Log parsing failures**: Use `ESP_LOGW()` when validation fails
- **Check for NaN**: Use `std::isnan(x)` for floats

**Example** (RGB validation):
```yaml
on_value:
  - lambda: |-
      int r, g, b;
      if (sscanf(x.c_str(), "[%d,%d,%d]", &r, &g, &b) == 3) {
        r = r < 0 ? 0 : (r > 255 ? 255 : r);  // Clamp
        g = g < 0 ? 0 : (g > 255 ? 255 : g);
        b = b < 0 ? 0 : (b > 255 ? 255 : b);
        id(color_val) = (r << 16) | (g << 8) | b;
      } else {
        ESP_LOGW("rgb", "Bad format: %s", x.c_str());
      }
```

---

## 6. C++ Headers & Performance

**C++ Headers** (`src/*.h`):
- Always validate pointers before use
- Use `static_cast<>`, not C-style casts  
- Pre-allocate containers with `reserve()`
- Check `user_data` for nullptr before casting
- Use ESP logging: `ESP_LOGD/I/W/E("tag", "message")`

**Performance Tips**:
- Chart buffers: ~480 bytes (acceptable on ESP32-S3)
- Don't use `std::deque` in globals (not supported)
- Update frequencies: Lights (on change), Sensors (5-60s), Clock (1s)
- Skip updates when display off: `if (!id(backlight).remote_values.is_on()) return;`

---

## 7. Quick Example: Adding a Widget

**To add a humidity sensor to 4-tabs layout:**

1. **Global**: Add `my_humidity_val` to `templates/core/globals.yaml`
2. **Data Bridge**: Add HA sensor to `layouts/4-tabs/data_bridge.yaml` → updates global → calls refresh script
3. **Refresh Script**: Add `my_humidity_refresh` to `layouts/4-tabs/widget_logic.yaml` (with nullptr check)
4. **Widget**: Create `templates/widgets/humidity.yaml` with `${widget_id}` and `${widget_id}_label`
5. **Use Widget**: Include in `layouts/4-tabs/tabs/overview.yaml` with vars
6. **Config**: Add `my_humidity_entity` substitution to `testing.yaml`
7. **Stub (if optional)**: Uncomment in `layouts/4-tabs/id_stubs.yaml`
8. **Test**: `esphome compile testing.yaml`

---

## 8. Style & UI Guidelines
*   Use `${grid_gap}` and `${grid_padding}` for alignment.
*   Reuse styles from `theme/style.yaml` via `style_definitions`.
*   Keep icons consistent using MDI hex codes (e.g., `"\U000F0335"`).
*   Follow the color scheme defined in `theme/defaults.yaml`.
*   Use consistent font sizes: `${font_id_small}`, `${font_id}`, `${font_id_large}`.
