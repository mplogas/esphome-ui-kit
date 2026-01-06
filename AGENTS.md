# Agent Instructions: ESPHome UI Kit

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

---

## 1. The 3-View Architecture (Strict Separation)

You must maintain the decoupling between these layers at all costs.

### View 1: User Configuration (`testing.yaml`, `mapping-defaults.yaml`)
*   **Role**: Assigns widgets to slots using substitutions.
*   **Rule**: Never add logic here. Only definitions.
*   **Pattern**: Uses `tab_[N]_slot_[X]_[Y]_widget` to point to a file in `templates/`.

### View 2: Design & Widgets (`templates/tiles/`, `templates/widgets/`, `theme/`)
*   **Role**: Visual representation.
*   **Rule**: Must be **Data-Aware** (use global variables) but **Layout-Agnostic** (don't assume where they are placed).
*   **Implementation**: Use `vars` passed from View 1 to set internal IDs and entities.
*   **Naming Convention**: Use `${id}` passed via vars for the root object ID of a widget.

### View 3: Core Logic (`templates/core/`, `templates/scripts/`)
*   **Role**: The "Data Bridge". Fetches data from Home Assistant and updates Global Variables.
*   **Rule**: **NEVER** reference an LVGL object ID directly unless you use a check or a `stubs` system.
*   **The Bridge Pattern**: 
    1. HA Sensor -> `data_bridge.yaml` 
    2. `data_bridge.yaml` -> Updates `globals.yaml` 
    3. `globals.yaml` -> Widget (View 2) reads this to update itself.

---

## 2. Preventing "Fix-Error-Loop" & Crashes

### Rule A: The "Stub" Requirement
If you create a new widget that is updated via a script (e.g., a new graph), you **MUST** add a corresponding stub in `templates/core/id_stubs.yaml`. 
*   **Why**: ESPHome refresh scripts will fail to compile if they reference an ID that isn't in the current layout. Stubs ensure the ID always exists in a hidden page.

### Rule B: Global Variable Fallbacks
Always initialize global variables with safe defaults in `globals.yaml`. Use `nan` for floats or empty strings for text to avoid rendering garbage before HA connects.

### Rule C: Decoupling Check
If deleting a widget breaks the build, it means you have a **"Hard Link"** in your logic. 
*   **Fix**: Move the logic that references that widget's ID into a script, or use a lambda check if possible.

---

## 3. Workflow for New Features

1.  **Define Globals**: Add necessary variables to `templates/core/globals.yaml`.
2.  **Implement Data Bridge**: Add the HA sensor/binary_sensor to `templates/core/data_bridge.yaml`. Update the global variable on state change.
3.  **Create/Modify Widget**: Update `templates/tiles/` or `widgets/`. Use the global variables for values.
4.  **Update Stubs**: Add any mandatory IDs to `templates/core/id_stubs.yaml`.
5.  **Configure in YAML**: Map the widget in `testing.yaml` (or `mapping-defaults.yaml` for defaults).
6.  **Compile & Validate**: Run `esphome compile testing.yaml`.

## 4. Troubleshooting
### Substitution Cascading
ESPHome `!include` with `vars` does NOT automatically pass variables down multiple levels of nesting. 
*   If `testing.yaml` includes `layout.yaml` with `vars: { my_var: "val" }`, and `layout.yaml` includes `widget.yaml`, the `widget.yaml` **will not see** `my_var` unless `layout.yaml` explicitly passes it: `vars: { my_var: "${my_var}" }`.

### Common Error Analysis
*   **"Component not found"**: Usually a missing `!include` in the root yaml.
*   **"ID already defined"**: You likely included a template twice or defined a stub that already exists in the layout.
*   **Indentation errors**: ESPHome's multi-level includes with `vars` are extremely sensitive. Double-check spaces.

---

## 5. Style & UI Guidelines
*   Use `${grid_gap}` and `${grid_padding}` for alignment.
*   Reuse styles from `theme/style.yaml` via `style_definitions`.
*   Keep icons consistent using MDI hex codes (e.g., `"\U000F0335"`).
