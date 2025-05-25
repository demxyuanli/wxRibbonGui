#ifndef FLATUI_CONSTANTS_H
#define FLATUI_CONSTANTS_H

#include <wx/wx.h>
#include <wx/settings.h> // For wxSystemSettings

// Global default background color (light gray)
// This will now be the primary content background for active tabs, panels, etc.
const wxColour FLATUI_PRIMARY_CONTENT_BG_COLOUR(240, 240, 2430); // Deeper light gray
static const wxColour FLATUI_DEFAULT_BG_COLOUR(FLATUI_PRIMARY_CONTENT_BG_COLOUR);

// Global default border color (gray)
static const wxColour FLATUI_DEFAULT_BORDER_COLOUR(wxGREY_PEN->GetColour());

// Default Colors
static const wxColour FLATUI_DEFAULT_TEXT_COLOUR(*wxBLACK);
static const wxColour FLATUI_HIGHLIGHT_COLOUR(0, 120, 215);      // Example highlight color (blueish)

// Default Font Settings
// These constants are no longer directly used by GetFlatUIDefaultFont() if returning system default.
// They can be kept for reference or other potential uses.
const int FLATUI_DEFAULT_FONT_SIZE = 6;
const wxFontFamily FLATUI_DEFAULT_FONT_FAMILY = wxFONTFAMILY_SWISS;
const wxFontStyle FLATUI_DEFAULT_FONT_STYLE = wxFONTSTYLE_NORMAL;
const wxFontWeight FLATUI_DEFAULT_FONT_WEIGHT = wxFONTWEIGHT_NORMAL;
static const wxString FLATUI_DEFAULT_FONT_FACE_NAME = wxS("Consolas");

// Helper to get the default font
static wxFont GetFlatUIDefaultFont()
{
    // Return the system's default GUI font
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

// --- FlatUIBar Specific Colors ---
const wxColour FLATUI_BAR_BACKGROUND_COLOUR(220, 220, 220); // Bar strip background
const wxColour FLATUI_ACT_BAR_BACKGROUND_COLOUR(*wxWHITE); // Bar strip background
// const wxColour FLATUI_BAR_ACTIVE_TAB_COLOUR(245, 245, 245); // Replaced by FLATU
// I_PRIMARY_CONTENT_BG_COLOUR
// const wxColour FLATUI_BAR_INACTIVE_TAB_COLOUR(205, 210, 215); // No longer used for fill
const wxColour FLATUI_BAR_TAB_BORDER_COLOUR(170, 170, 170);     // For active tab left/right borders
const wxColour FLATUI_BAR_ACTIVE_TAB_TOP_BORDER_COLOUR(180, 180, 180); // For active tab top border (2px)
const wxColour FLATUI_BAR_ACTIVE_TEXT_COLOUR(*wxBLACK);
const wxColour FLATUI_BAR_INACTIVE_TEXT_COLOUR(70, 70, 70);

// --- FlatUIHomeSpace Specific Colors ---
const wxColour FLATUI_HOMESPACE_HOVER_BG_COLOUR(200, 200, 200); // Gray for hover

// --- FlatUIPanel Specific Colors ---
const wxColour FLATUI_PANEL_BORDER_COLOUR(200, 200, 200);
const wxColour FLATUI_PANEL_HEADER_COLOUR(230, 230, 230);
const wxColour FLATUI_PANEL_HEADER_TEXT_COLOUR(*wxBLACK);

// --- Sizes & Spacing ---
// FlatUIBar
const int FLATUI_BAR_RENDER_HEIGHT = 26; // Renamed from FLATUI_BAR_HEIGHT to avoid conflict with FlatUIBar::GetBarHeight() static method, used for rendering calculations.
const int FLATUI_BAR_TAB_PADDING = 10;
const int FLATUI_BAR_TAB_SPACING = 0;
const int FLATUI_BAR_ELEMENT_SPACING = 5;
const int FLATUI_BAR_BAR_PADDING = 0; // Renamed from FLATUI_BAR_PADDING to avoid conflict with a potential variable

// FlatUIButtonBar
const int FLATUI_BUTTONBAR_TARGET_HEIGHT = 30;
const int FLATUI_BUTTONBAR_ICON_SIZE = 16;
const int FLATUI_BUTTONBAR_HORIZONTAL_PADDING = 4;
// Vertical padding calculated based on height and icon size, so not a direct constant here unless fixed.
// const int FLATUI_BUTTONBAR_INTERNAL_VERTICAL_PADDING = (FLATUI_BUTTONBAR_TARGET_HEIGHT - FLATUI_BUTTONBAR_ICON_SIZE) / 2;
const int FLATUI_BUTTONBAR_SPACING = 2;
const int FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN = 2;

// FlatUIGallery
const int FLATUI_GALLERY_TARGET_HEIGHT = 30;
const int FLATUI_GALLERY_ITEM_SPACING = 2;
const int FLATUI_GALLERY_HORIZONTAL_MARGIN = 2;

// FlatUIPanel
const int FLATUI_PANEL_INTERNAL_VERTICAL_PADDING = 2; // Top and bottom padding inside panels
const int FLATUI_PANEL_INTERNAL_HORIZONTAL_PADDING = 2; // Left and right padding inside panels
const int FLATUI_PANEL_INTERNAL_PADDING_TOTAL = FLATUI_PANEL_INTERNAL_HORIZONTAL_PADDING * 2; // Total horizontal padding
const int FLATUI_PANEL_TARGET_HEIGHT = 60; // Increased from 65 to accommodate content and headers
const int FLATUI_PANEL_COLLAPSED_HEIGHT = 25; // Panels in collapsed state
const int FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE = 20; // For TOP or LEFT headers

// FlatUIButtonBar Style Defaults
const int FLATUI_BUTTONBAR_DEFAULT_BORDER_WIDTH = 1;
const int FLATUI_BUTTONBAR_DEFAULT_CORNER_RADIUS = 4;
const wxColour FLATUI_BUTTONBAR_DEFAULT_BG_COLOUR(240, 240, 240);
const wxColour FLATUI_BUTTONBAR_DEFAULT_HOVER_BG_COLOUR(230, 230, 230);
const wxColour FLATUI_BUTTONBAR_DEFAULT_PRESSED_BG_COLOUR(220, 220, 220);
const wxColour FLATUI_BUTTONBAR_DEFAULT_TEXT_COLOUR(*wxBLACK);
const wxColour FLATUI_BUTTONBAR_DEFAULT_BORDER_COLOUR(200, 200, 200);
const wxColour FLATUI_BUTTONBAR_PANEL_BG_COLOUR(230, 230, 230);
const wxColour FLATUI_BUTTONBAR_PANEL_BORDER_COLOUR(200, 200, 200);

#endif // FLATUI_CONSTANTS_H