#ifndef FLATUIBAR_CONFIG_H
#define FLATUIBAR_CONFIG_H

struct FlatUIBarConfig {
    // Layout constants
    static constexpr int FIXED_PANEL_Y = 30;
    static constexpr int PIN_BUTTON_MARGIN = 4;
    static constexpr int ELEMENT_SPACING = 5;
    static constexpr int BAR_PADDING = 2;
    static constexpr int TAB_PADDING = 10;
    static constexpr int TAB_SPACING = 1;
    
    // Timing constants
    static constexpr int AUTO_HIDE_DELAY_MS = 500;
    
    // Size constants
    static constexpr int CONTROL_WIDTH = 20;
    static constexpr int CONTROL_HEIGHT = 20;
    static constexpr int MIN_PANEL_WIDTH = 100;
    static constexpr int MIN_PANEL_HEIGHT = 50;
    
    // Visual constants
    static constexpr int SHADOW_OFFSET = 1;
    static constexpr int BORDER_WIDTH = 1;
    static constexpr int CORNER_RADIUS = 4;
    
    // Interaction constants
    static constexpr int MOUSE_MARGIN = 5;
    static constexpr int DRAG_THRESHOLD = 3;
};

#endif // FLATUIBAR_CONFIG_H 