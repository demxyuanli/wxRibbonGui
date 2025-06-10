#ifndef FLATUIBAR_H
#define FLATUIBAR_H

#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUISpacerControl.h"
#include <wx/wx.h>
#include <wx/vector.h>
#include <string> // Keep if std::string is used, though not visible here
#include <wx/artprov.h>

// Forward declarations of the new component classes
class FlatUIPage; 
class FlatUIHomeSpace;
class FlatUIFunctionSpace;
class FlatUIProfileSpace;
class FlatUISystemButtons;
class FlatUISpacerControl;  

class FlatUIBar : public wxControl
{
public:
    // FLATUI_BAR_HEIGHT is now FLATUI_BAR_RENDER_HEIGHT in FlatUIConstants.h for paint calcs.
    // The GetBarHeight() static method remains important for overall height logic.
    // static const int FLATUI_BAR_HEIGHT = 30; // Removed, use constant from FlatUIConstants.h for rendering logic
    
    FlatUIBar(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~FlatUIBar();

    // --- Configuration Methods ---
    // Home Button (Dropdown Menu Icon)
    void SetHomeButtonMenu(wxMenu* menu);
    void SetHomeButtonIcon(const wxBitmap& icon = wxNullBitmap);
    void SetHomeButtonWidth(int width);

    // Page Tabs Management (remains directly managed by FlatUIBar for now)
    void AddPage(FlatUIPage* page);
    void SetActivePage(size_t index);
    size_t GetPageCount() const;
    size_t GetActivePage() const { return m_activePage; }
    FlatUIPage* GetPage(size_t index) const;
    
    // Tab Style Configuration
    enum class TabStyle {
        DEFAULT,        // Default style with top border for active tab
        UNDERLINE,      // Underline style for active tab
        BUTTON,         // Button-like appearance
        FLAT            // Completely flat, only text color changes
    };
    
    // Tab Border Style
    enum class TabBorderStyle {
        SOLID,          // Solid line border
        DASHED,         // Dashed line border
        DOTTED,         // Dotted line border
        DOUBLE,         // Double line border
        GROOVE,         // Groove style border
        RIDGE,          // Ridge style border
        ROUNDED         // Rounded corners
    };
    
    void SetTabStyle(TabStyle style);
    TabStyle GetTabStyle() const { return m_tabStyle; }
    
    // Tab border configuration
    void SetTabBorderStyle(TabBorderStyle style);
    TabBorderStyle GetTabBorderStyle() const { return m_tabBorderStyle; }
    void SetTabBorderWidths(int top, int bottom, int left, int right);
    void GetTabBorderWidths(int& top, int& bottom, int& left, int& right) const;
    void SetTabBorderColour(const wxColour& colour);
    wxColour GetTabBorderColour() const { return m_tabBorderColour; }
    
    // Individual border color configuration
    void SetTabBorderTopColour(const wxColour& colour);
    wxColour GetTabBorderTopColour() const { return m_tabBorderTopColour; }
    void SetTabBorderBottomColour(const wxColour& colour);
    wxColour GetTabBorderBottomColour() const { return m_tabBorderBottomColour; }
    void SetTabBorderLeftColour(const wxColour& colour);
    wxColour GetTabBorderLeftColour() const { return m_tabBorderLeftColour; }
    void SetTabBorderRightColour(const wxColour& colour);
    wxColour GetTabBorderRightColour() const { return m_tabBorderRightColour; }
    
    // Individual border width configuration
    void SetTabBorderTopWidth(int width);
    int GetTabBorderTopWidth() const { return m_tabBorderTop; }
    void SetTabBorderBottomWidth(int width);
    int GetTabBorderBottomWidth() const { return m_tabBorderBottom; }
    void SetTabBorderLeftWidth(int width);
    int GetTabBorderLeftWidth() const { return m_tabBorderLeft; }
    void SetTabBorderRightWidth(int width);
    int GetTabBorderRightWidth() const { return m_tabBorderRight; }
    
    void SetTabCornerRadius(int radius);  // For rounded style
    int GetTabCornerRadius() const { return m_tabCornerRadius; }
    
    // Tab colors
    void SetActiveTabBackgroundColour(const wxColour& colour);
    wxColour GetActiveTabBackgroundColour() const { return m_activeTabBgColour; }
    void SetActiveTabTextColour(const wxColour& colour);
    wxColour GetActiveTabTextColour() const { return m_activeTabTextColour; }
    void SetInactiveTabTextColour(const wxColour& colour);
    wxColour GetInactiveTabTextColour() const { return m_inactiveTabTextColour; }

    // Custom Spaces
    void SetFunctionSpaceControl(wxWindow* funcControl, int width = -1);
    void SetProfileSpaceControl(wxWindow* profControl, int width = -1);
    void ToggleFunctionSpaceVisibility();
    void ToggleProfileSpaceVisibility();
    void SetFunctionSpaceCenterAlign(bool center);
    void SetProfileSpaceRightAlign(bool rightAlign);
    
    void SetTabFunctionSpacerAutoExpand(bool autoExpand);
    
    void SetFunctionProfileSpacerAutoExpand(bool autoExpand);
    
    // New unified spacer management
    enum SpacerPosition {
        SPACER_BEFORE,
        SPACER_AFTER
    };
    
    enum SpacerLocation {
        SPACER_TAB_FUNCTION,      // Between tabs and function space
        SPACER_FUNCTION_PROFILE   // Between function and profile space
    };
    
    void AddSpaceSeparator(SpacerLocation location, int width, bool drawSeparator = false, bool canDrag = true, bool autoExpand = false);

    static int GetBarHeight(); // Renamed from GetTabAreaHeight for clarity

    // Bar margin configuration
    void SetBarTopMargin(int margin);
    int GetBarTopMargin() const { return m_barTopMargin; }

    void SetBarBottomMargin(int margin);
    int GetBarBottomMargin() const noexcept { return m_barBottomMargin; }

    void SetBarBottomMargin(int margin);
    int GetBarBottomMargin() const noexcept { return m_barBottomMargin; }

    // Override to provide best size hint
    virtual wxSize DoGetBestSize() const override;

    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnMouseDown(wxMouseEvent& evt); // Will primarily handle tab clicks now
    // OnMouseMove and OnMouseLeave might be less relevant here if sub-controls handle their own hover


    FlatUISpacerControl* GetTabFunctionSpacer() { return m_tabFunctionSpacer; }
    FlatUISpacerControl* GetFunctionProfileSpacer() { return m_functionProfileSpacer; }


    FlatUIHomeSpace* GetHomeSpace() { return m_homeSpace; }

private:

    void OnShow(wxShowEvent& event);

    // --- Child Component Controls ---
    FlatUIHomeSpace* m_homeSpace;
    // TabSpace is currently handled directly by FlatUIBar's m_pages and PaintTabs
    FlatUIFunctionSpace* m_functionSpace;
    FlatUIProfileSpace* m_profileSpace;
    FlatUISystemButtons* m_systemButtons;
    

    FlatUISpacerControl* m_tabFunctionSpacer;    
    FlatUISpacerControl* m_functionProfileSpacer; 

    // --- Page Tabs elements (directly managed) ---
    wxVector<FlatUIPage*> m_pages;
    size_t m_activePage;
    wxRect m_tabAreaRect; // Stores the calculated rectangle for drawing tabs
    
    // Tab style configuration
    TabStyle m_tabStyle;
    TabBorderStyle m_tabBorderStyle;

    int m_tabTopSpacing;
    int m_tabBorderTop;
    int m_tabBorderBottom;
    int m_tabBorderLeft;
    int m_tabBorderRight;
    int m_tabCornerRadius;
    wxColour m_tabBorderColour;
    wxColour m_tabBorderTopColour;
    wxColour m_tabBorderBottomColour;
    wxColour m_tabBorderLeftColour;
    wxColour m_tabBorderRightColour;
    wxColour m_activeTabBgColour;
    wxColour m_activeTabTextColour;
    wxColour m_inactiveTabTextColour;
    
    // Bar margin
    int m_barTopMargin;
    int m_barBottomMargin;
    
    // --- Configuration for direct Tab drawing by FlatUIBar ---
    // static const int TAB_PADDING = 10; // Removed
    // static const int TAB_SPACING = 1;  // Removed
    
    // --- General Layout Constants (can be adjusted) ---
    // static const int ELEMENT_SPACING = 5; // Removed
    // static const int BAR_PADDING = 2;     // Removed

    // --- Helper methods ---
    void UpdateElementPositionsAndSizes(const wxSize& barSize);
    void PaintTabs(wxDC& dc, int availableWidth, int& currentXOffset); // For drawing tabs directly
    int CalculateTabsWidth(wxDC& dc) const; // For calculating width needed by direct tabs
    void DrawTabBorder(wxDC& dc, const wxRect& tabRect, bool isActive); // Draw tab border with style

    // GetTopLevelFrame might now be primarily used by m_systemButtons internally
    // wxFrame* GetTopLevelFrame() const; 

    void DrawBackground(wxDC& dc);
    void DrawBarSeparator(wxDC& dc);
    void PaintTabsArea(wxDC& dc, int availableWidth, int& currentXOffset);
    void HandleTabAreaClick(const wxPoint& pos);

    bool m_functionSpaceCenterAlign;
    bool m_profileSpaceRightAlign;
};

#endif // FLATUIBAR_H 