**Goal:** Lightweight abstraction over ESPHome's LVGL feature. I want to be able to quickly set up UI elements for various ESPHome devices with displays (mostly LCD, some ePaper)
**Requirements:**
1. A tabview is always going to be my "root layout", as it brings swiping and navigation.
    - I want to be able to override the defaults of tab_style via parameters in the config
    - I want to be able to control the postion of the tabs
    - I want to be able to controle the name, the id and the widgets of a tab
2. The upmost item in each tab should be an object (obj) that is a container for all.
    - default align should be center, but i want to be able to override it
    - it should use the grid layout, with a default 2x2 grid, but i want to be able to configure that
3. I want to be able to add my own widgets to each container objects
    - I also want to be able to assign premade sets of widgets to a container object, like a weather Widget or a clock widget. 
    - common properties (font color or bg_color) of these premade widgets should be configurable from their defaults 
4. provide a simple way of abstracting hardware, e.g. by allowing "hardware templates" I have multiple SenseCap Indicator devices, and I want to have comparable small esphome config files for each of them.

**Links:**
- EspHome LVGL overview: https://esphome.io/components/lvgl/
- EspHome LVLG Widgets (includes definition for all widgets) : https://esphome.io/components/lvgl/widgets/
- ESPHome LVGL Widtgets Tabview:  https://esphome.io/components/lvgl/widgets/#tabview 
- EspHome Tips & Tricks and Examples: https://esphome.io/cookbook/lvgl/
- espHome Grid Layout positioning: https://esphome.io/cookbook/lvgl/#grid-layout-positioning
- Sensecap Indicator EspHome sample config: https://devices.esphome.io/devices/seeed-sensecap/