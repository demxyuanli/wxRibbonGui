#ifndef FLATUI_CONSTANTS_H
#define FLATUI_CONSTANTS_H

#include <wx/wx.h>
#include <wx/settings.h>

// Global Colors
const wxColour FLATUI_PRIMARY_FRAME_BORDER_COLOUR(230, 200, 0);
const wxColour FLATUI_PRIMARY_CONTENT_BG_COLOUR(250, 250, 250); // Primary background for active content
const wxColour FLATUI_DEFAULT_BG_COLOUR(FLATUI_PRIMARY_CONTENT_BG_COLOUR);
const wxColour FLATUI_DEFAULT_BORDER_COLOUR(170, 170, 170);
const wxColour FLATUI_DEFAULT_TEXT_COLOUR(100, 100, 100);
const wxColour FLATUI_HIGHLIGHT_COLOUR(0, 120, 215); // Blueish highlight

const wxColour FLATUI_WINDOW_MOTION_COLOR(150, 150, 150);
const wxColour FLATUI_FRAME_BORDER_COLOR(255, 100, 0);
const int SPACE_DEFAULT_WIDTH = 100;

// Default Font Settings
const int FLATUI_DEFAULT_FONT_SIZE = 8; // Adjusted for better readability
const wxFontFamily FLATUI_DEFAULT_FONT_FAMILY = wxFONTFAMILY_SWISS;
const wxFontStyle FLATUI_DEFAULT_FONT_STYLE = wxFONTSTYLE_NORMAL;
const wxFontWeight FLATUI_DEFAULT_FONT_WEIGHT = wxFONTWEIGHT_NORMAL;
const wxString FLATUI_DEFAULT_FONT_FACE_NAME = wxS("Consolas");

static wxFont GetFlatUIDefaultFont()
{
    return wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
}

// FlatUIBar Colors
const wxColour FLATUI_BAR_BACKGROUND_COLOUR(240, 240, 240);
const wxColour FLATUI_ACT_BAR_BACKGROUND_COLOUR(*wxWHITE);
const wxColour FLATUI_BAR_TAB_BORDER_COLOUR(170, 170, 170);
const wxColour FLATUI_BAR_ACTIVE_TAB_TOP_BORDER_COLOUR(180, 180, 180);
const wxColour FLATUI_BAR_ACTIVE_TEXT_COLOUR(100, 100, 100);
const wxColour FLATUI_BAR_INACTIVE_TEXT_COLOUR(70, 70, 70);

// FlatUIHomeSpace Colors
const wxColour FLATUI_HOMESPACE_HOVER_BG_COLOUR(200, 200, 200);

// FlatUIPanel Colors
const wxColour FLATUI_PANEL_BORDER_COLOUR(200, 200, 200);
const wxColour FLATUI_PANEL_SEPARATOR_COLOUR(200, 200, 200);
const wxColour FLATUI_PANEL_HEADER_COLOUR(230, 230, 230);
const wxColour FLATUI_PANEL_HEADER_TEXT_COLOUR(100, 100, 100);

// FlatUIButtonBar Colors
const wxColour FLATUI_BUTTONBAR_DEFAULT_BG_COLOUR(240, 240, 240);
const wxColour FLATUI_BUTTONBAR_DEFAULT_HOVER_BG_COLOUR(230, 230, 230);
const wxColour FLATUI_BUTTONBAR_DEFAULT_PRESSED_BG_COLOUR(220, 220, 220);
const wxColour FLATUI_BUTTONBAR_DEFAULT_TEXT_COLOUR(100, 100, 100);
const wxColour FLATUI_BUTTONBAR_DEFAULT_BORDER_COLOUR(200, 200, 200);
const wxColour FLATUI_BUTTONBAR_PANEL_BG_COLOUR(230, 230, 230);
const wxColour FLATUI_BUTTONBAR_PANEL_BORDER_COLOUR(200, 200, 200);

// Sizes & Spacing
// FlatUIBar
const int FLATUI_BAR_TOP_MARGIN = 2;
const int FLATUI_BAR_BOTTOM_MARGIN = 0;
const int FLATUI_BAR_RENDER_HEIGHT = 30;
const int FLATUI_BAR_TAB_PADDING = 10;
const int FLATUI_BAR_TAB_SPACING = 0;
const int FLATUI_BAR_ELEMENT_SPACING = 5;
const int FLATUI_BAR_PADDING = 0;
const int FLATUI_BAR_TAB_BORDER_TOP = 2;

// FlatUIButtonBar
const int FLATUI_BUTTONBAR_TARGET_HEIGHT = 30;
const int FLATUI_BUTTONBAR_ICON_SIZE = 24;
const int FLATUI_BUTTONBAR_HORIZONTAL_PADDING = 2;
const int FLATUI_BUTTONBAR_INTERNAL_VERTICAL_PADDING = (FLATUI_BUTTONBAR_TARGET_HEIGHT - FLATUI_BUTTONBAR_ICON_SIZE) / 2;
const int FLATUI_BUTTONBAR_SPACING = 2;
const int FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN = 2;
const int FLATUI_BUTTONBAR_DEFAULT_BORDER_WIDTH = 0;
const int FLATUI_BUTTONBAR_DEFAULT_CORNER_RADIUS = 0;
const int FLATUI_BUTTONBAR_MENU_VERTICAL_OFFSET = 2; // Offset for popup menu
const int FLATUI_BUTTONBAR_DROPDOWN_ARROW_WIDTH = 5; // Dropdown arrow width
const int FLATUI_BUTTONBAR_DROPDOWN_ARROW_HEIGHT = 3; // Dropdown arrow height

// New separator constants
const int FLATUI_BUTTONBAR_SEPARATOR_WIDTH = 1; // 1-pixel wide line
const int FLATUI_BUTTONBAR_SEPARATOR_PADDING = 2; // Padding on each side
const int FLATUI_BUTTONBAR_SEPARATOR_MARGIN = 4; // Top/bottom margin for aesthetics

// FlatUIGallery
const int FLATUI_GALLERY_TARGET_HEIGHT = 30;
const int FLATUI_GALLERY_ITEM_SPACING = 2;
const int FLATUI_GALLERY_HORIZONTAL_MARGIN = 2;

// FlatUIPanel
const int FLATUI_PANEL_INTERNAL_VERTICAL_PADDING = 2;
const int FLATUI_PANEL_INTERNAL_HORIZONTAL_PADDING = 2;
const int FLATUI_PANEL_INTERNAL_PADDING_TOTAL = FLATUI_PANEL_INTERNAL_HORIZONTAL_PADDING * 2;
const int FLATUI_PANEL_TARGET_HEIGHT = 60;
const int FLATUI_PANEL_COLLAPSED_HEIGHT = 30;
const int FLATUI_PANEL_DEFAULT_HEADER_AREA_SIZE = 20;
const int FLATUI_INNERBAR_BORDER_SPACING = 5;
const int FLATUI_PANEL_SEPARATOR_WIDTH = 1;

// FlatUIPanel/Page Margin & Padding
const int FLATUI_PANEL_MARGIN = 8;
const int FLATUI_PANEL_PADDING = 8;
const int FLATUI_PAGE_MARGIN = 8;
const int FLATUI_PAGE_PADDING = 8;

//SysButtons params

const int SYS_BUTTON_WIDTH = 40;
const int SYS_BUTTON_HEIGHT = 30;
const int SYS_BUTTON_SPACING = 0;

//HomeMenu Params
const int HOMEMENU_HEIGHT = 25; 
const int HOMEMENU_SEPARATOR_HEIGHT = 5;
const int HOMEMENU_WIDTH = 240;

#endif // FLATUI_CONSTANTS_H