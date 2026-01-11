# Agent Instructions: ESPHome UI Kit

Refer to [INSTRUCTIONS.md](INSTRUCTIONS.md) for the architecture philosophy and design vision. This document focuses on practical ESPHome constraints, compilation workflows, and troubleshooting.

## Quick Reference
- **Architecture Vision**: Consult [INSTRUCTIONS.md - The 3-View Approach](INSTRUCTIONS.md#architecture-the-3-view-approach).
- **Project Goals**: Consult [INSTRUCTIONS.md - Goal](INSTRUCTIONS.md#goal).
- **Document Scope**: ESPHome constraints, compilation, debugging, and security.

---

## Technical Environment
Before executing ESPHome commands, activate the virtual environment:
```bash
. ~/esphome/.venv/bin/activate
```
**Terminal Usage**: If a "zsh: correct..." prompt appears, disable it with `unsetopt correct` or utilize `python3 -m esphome`.

To verify modifications, execute:
```bash
esphome compile testing.yaml
```
Note: `testing.yaml` is the primary development file. Update `main-dashboard.yaml` only after a feature is verified as stable.

### Rule D: Configuration Preservation
Do not generate or write a root configuration file from scratch.
*   Utilize the existing `testing.yaml`.
*   Maintain the existing `substitutions`, `wifi`, `api`, `ota`, and `time` blocks.
*   Add packages or globals to the respective existing `packages:` or `substitutions:` blocks.

### Rule E: Terminal Context
Prioritize checking the terminal context before utilizing `run_terminal_command`. Verify terminal output if the execution environment is unclear.

---

## 0. ESPHome Platform Constraints

ESPHome is a firmware configuration system rather than an application development framework.

### System Capabilities:
- YAML-based firmware configuration.
- C++ lambdas for custom logic.
- Script system with basic parameter support.
- Hardware specifications: approximately 4MB flash and 320KB RAM (ESP32-S3).
- Compilation produces permanent firmware.
- Support for custom C++ headers.

### System Limitations:
- No support for full C++ application OOP patterns.
- No dependency injection.
- No event bus or pub/sub libraries.
- No runtime reflection or dynamic object querying.
- No ID interpolation in lambdas (e.g., `id(${variable}_icon)` is unsupported).
- `globals:` supports `std::vector` but not `std::deque` or other custom containers.

### Technical Implications:

**1. ID Declaration Requirements**
ESPHome requires all `id()` references to be known at compile time.
```cpp
// This will not compile even with a null check if the ID is undeclared:
if (id(light_1_icon) == nullptr) return;
```

**2. ID Parameterization Limits**
ID references cannot be parameterized within scripts.
```yaml
# This implementation is not supported in ESPHome:
script:
  - id: refresh_light
    parameters:
      light_id: std::string
    then:
      - lambda: |-
          id(${light_id}_icon).set_color(...)
```

**3. Functional Redundancy**
Due to platform constraints, individual refresh scripts are often required for separate widgets (e.g., four separate `light_X_refresh` scripts).

**4. Compile-Time Stubs**
Stubs are utilized to satisfy the compiler and do not replace the need for runtime null checks.

### Layout Package Implications
Layout packages (comprising View 2 and View 3) couple refresh scripts to specific widget IDs. This is an intentional design choice to ensure self-contained packages.

---

## 1. The 3-View Architecture (Separation of Concerns)

Consult [INSTRUCTIONS.md - Architecture](INSTRUCTIONS.md#architecture-the-3-view-approach) for the conceptual vision. This section addresses practical implementation.

### View 1: User Configuration (`testing.yaml`, `main-dashboard.yaml`)
*   **Role**: Select a layout package and map Home Assistant entities.
*   **Rule**: Never add logic here. Only definitions and entity mappings.
*   **Pattern**: Include a layout package (e.g., `!include layouts/4-tabs/package.yaml`) and define entity substitutions.
*   **Files**: `testing.yaml`, `main-dashboard.yaml`

### View 2: Design & Widgets (`templates/widgets/`, `layouts/`, `theme/`)
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
    - **Universal Core** (`templates/core/globals.yaml`): State variables shared across ALL layouts.
    - **Layout-Specific** (`layouts/*/data_bridge.yaml`, `layouts/*/widget_logic.yaml`): Entity listeners and refresh scripts specific to a layout.
*   **Rule**: Layout packages explicitly declare their dependencies. Scripts reference View 2 IDs directly (with null checks for safety).
*   **Reality**: Tight coupling between a layout and its scripts is **intentional** - choosing a layout means accepting its dependencies.
*   **The Bridge Pattern**: 
    1. HA Sensor -> `layouts/X/data_bridge.yaml` 
    2. `data_bridge.yaml` -> Updates `templates/core/globals.yaml` 
    3. `globals.yaml` -> Widget (View 2) reads this to update itself
    4. `layouts/X/widget_logic.yaml` -> Refreshes UI from globals

---

## 2. Preventing Build Failures and Crashes

### Rule A: The "Stub" Requirement (Compile-Time Validation)

**The Problem:**
ESPHome validates all `id()` references during compilation, even those within conditional checks.

**Example:**
```cpp
// This will not compile if light_1_icon is undeclared:
if (id(light_1_icon) == nullptr) return; // Compile error: undeclared identifier
```

**The Solution:**
Stubs declare IDs in a hidden page (`layouts/X/id_stubs.yaml`) to satisfy the compiler.

**When to Use Stubs:**
1. A script references an ID that is not present in the active layout.
2. The compiler returns "undeclared identifier" errors.
3. Note: Null checks handle runtime safety; stubs handle compilation.

**Parameters:**
- Stubs consume approximately 10 bytes per object in the binary.
- Stubs facilitate flexible layouts without breaking shared scripts.
- Most stubs are commented out by default; uncomment as needed.

### Rule B: Global Variable Fallbacks
Initialize global variables with safe defaults in `globals.yaml`. Use `nan` for floats or empty strings for text to prevent incorrect rendering before Home Assistant connectivity is established.

### Rule C: Decoupling Check
If removing a widget causes compilation failures, a direct dependency exists in the logic.
*   **Fix**: Relocate the logic referencing the widget ID into a script or implement a lambda check.

---

## 3. Implementation Workflow

**Sequence**:

1.  **Define Globals**: Add variables to `templates/core/globals.yaml` (universal) or `layouts/X/globals_extension.yaml` (layout-specific).
2.  **Implement Data Bridge**: Add the Home Assistant sensor/binary_sensor to `layouts/X/data_bridge.yaml`. Update the global variable on state changes.
3.  **Create Refresh Script**: Add a script to `layouts/X/widget_logic.yaml` to update the UI from global state.
4.  **Develop or Modify Widget**: Update `templates/widgets/`. Utilize global variables for values.
5.  **Update Stubs**: Add mandatory IDs to `layouts/X/id_stubs.yaml` if necessary.
6.  **Configure Substitutions**: Map entities in `testing.yaml`.
7.  **Validate**: Execute `esphome compile testing.yaml`.
8.  **Deploy**: Upload to the device and verify functionality.

Refer to Section 8 for a detailed implementation example.

## 4. Troubleshooting

### Compilation Failures

#### "Undeclared identifier 'light_1_icon'"
**Cause**: A script in `layouts/X/widget_logic.yaml` references an ID not present in the active layout.

**Fix**: Uncomment the corresponding stub in `layouts/X/id_stubs.yaml`:
```yaml
# Uncomment to enable stub:
- label: { id: light_1_icon }
```

**Note**: ESPHome validates IDs at compile time, including those within `if (id(foo) == nullptr)` checks.

---

#### "Component already defined" or "ID already defined"
**Cause**: Duplicate `!include` statements or a stub conflict with a layout object.

**Fix**: 
1. Search for duplicate `id: foo` declarations.
2. Verify if a stub and a real widget share the same ID.
3. Remove the redundant declaration or comment out the stub.

---

#### "Component not found"
**Cause**: Missing `!include` in the root YAML or an incorrect file path.

**Fix**:
1. Verify all required includes are present in the `packages:` block.
2. Confirm relative file paths.
3. Check for filename typographical errors.

---

#### Indentation Errors
**Cause**: Spacing sensitivities in multi-level includes with `vars`.

**Fix**: 
- Utilize a YAML validator.
- Maintain consistent two-space indentation.
- Verify `vars:` block alignment.

---

### Substitution Cascading

**Issue**: ESPHome `!include` statements with `vars` do not automatically propagate variables to deeper inclusion levels.

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
    # widget.yaml cannot access my_var without explicit passing
```

**Fix**: Explicitly pass variables at each level:
```yaml
# layout.yaml
- !include
    file: widget.yaml
    vars:
      my_var: "${my_var}" # Variable is passed through
```

---

### Runtime Issues

#### Color Values Initialization
**Cause**: Global variables initialize to 0 before Home Assistant connectivity is established.

**Fix**: This is expected behavior. The `data_bridge.yaml` updates values upon connection. Consider defining default colors in `globals.yaml`:
```yaml
- id: light_1_color_val
  type: uint32_t
  initial_value: '0xFFFFFF' # Default to white
```

---

#### Chart Data Initialization
**Cause**: Chart container object is null or initialization failed.

**Fix**:
1. Review ESPHome logs for "Chart user_data is null" warnings.
2. Confirm `init_graph` execution in `esphome.on_boot`.
3. Verify the existence of the `{widget_id}_chart_container` object in the layout.

---

#### UI Responsiveness
**Cause**: Refresh script execution failures or Home Assistant update delays.

**Debugging**:
1. Add logging to refresh scripts:
   ```yaml
   - lambda: |-
       ESP_LOGD("refresh", "light_1_refresh called");
   ```
2. Verify Home Assistant entity updates.
3. Confirm entity IDs in the substitutions block.

---

#### Modification Impact
**Cause**: Deleting a widget may break logic that still references its ID.

**Resolution**:
1. Retain the script; null checks will prevent crashes.
2. Comment out the script in `layouts/X/widget_logic.yaml`.
3. Enable stubs for the IDs in `layouts/X/id_stubs.yaml`.

Note: Layout packages explicitly define dependencies, and this coupling is intentional.

---

## 5. Data Validation and Security

Validate all Home Assistant data to ensure system stability.

**Guidelines**:
- **Constraint checking**: Clamp numeric values (e.g., RGB 0-255, percentages 0-100).
- **Null verification**: Use `if (id(my_label) == nullptr) return;`.
- **Error logging**: Use `ESP_LOGW()` for validation failures.
- **Floating point validation**: Use `std::isnan(x)` for float types.

**Example**:
```yaml
on_value:
  - lambda: |-
      int r, g, b;
      if (sscanf(x.c_str(), "[%d,%d,%d]", &r, &g, &b) == 3) {
        r = r < 0 ? 0 : (r > 255 ? 255 : r); // Value clamping
        g = g < 0 ? 0 : (g > 255 ? 255 : g);
        b = b < 0 ? 0 : (b > 255 ? 255 : b);
        id(color_val) = (r << 16) | (g << 8) | b;
      } else {
        ESP_LOGW("rgb", "Invalid format: %s", x.c_str());
      }
```

---

## 6. Performance and Implementation

**C++ Headers**:
- Validate pointers before access.
- Utilize `static_cast<>` instead of C-style casts.
- Pre-allocate containers utilizing `reserve()`.
- Verify `user_data` pointer before casting.
- Utilize ESP logging: `ESP_LOGD/I/W/E("tag", "message")`.

**Optimization**:
- Chart buffers consume approximately 480 bytes (compatible with ESP32-S3).
- Utilize `std::vector` for globals; `std::deque` is not supported.
- Update frequencies: Lights (on state change), Sensors (5 to 60 seconds), Clock (1 second).
- Suspend updates when the display is inactive: `if (!id(backlight).remote_values.is_on()) return;`.

---

## 7. Configuration Example

**Adding a humidity sensor to the 4-tabs layout**:

1. **Global State**: Add `my_humidity_val` to `templates/core/globals.yaml`.
2. **Data Bridge**: Add the Home Assistant sensor to `layouts/4-tabs/data_bridge.yaml`.
3. **Refresh Script**: Add `my_humidity_refresh` to `layouts/4-tabs/widget_logic.yaml` (include null verification).
4. **Widget Definition**: Create `templates/widgets/humidity.yaml` utilizing `${widget_id}` and `${widget_id}_label`.
5. **Layout Inclusion**: Include the widget in `layouts/4-tabs/tabs/overview.yaml` with appropriate variables.
6. **Substitution**: Add the `my_humidity_entity` substitution to `testing.yaml`.
7. **Stubs**: Enable the required stub in `layouts/4-tabs/id_stubs.yaml` if the widget is optional.
8. **Verification**: Execute `esphome compile testing.yaml`.

---

## 8. Style and Interface Guidelines
*   Utilize `${grid_gap}` and `${grid_padding}` for alignment consistency.
*   Inherit styles from `theme/style.yaml` via `style_definitions`.
*   Maintain icon consistency using MDI hexadecimal codes (e.g., `"\U000F0335"`).
*   Adhere to the color scheme defined in `theme/defaults.yaml`.
*   Maintain consistent font sizes: `${font_id_small}`, `${font_id}`, `${font_id_large}`.
