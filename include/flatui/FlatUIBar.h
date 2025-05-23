#ifndef FLATUIBAR_H
#define FLATUIBAR_H

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

    // Custom Spaces
    void SetFunctionSpaceControl(wxWindow* funcControl, int width = 0); // width = 0 uses control's default
    void SetProfileSpaceControl(wxWindow* profControl, int width = 0); // width = 0 uses control's default
    
    void SetTabFunctionSpacer(int width, bool drawSeparator = false, bool dragFlag = false);
    void SetFunctionProfileSpacer(int width, bool drawSeparator = false, bool dragFlag = false);
    
    void SetTabFunctionSpacerAutoExpand(bool autoExpand);
    
    void SetFunctionProfileSpacerAutoExpand(bool autoExpand);

    static int GetBarHeight(); // Renamed from GetTabAreaHeight for clarity

    // Override to provide best size hint
    virtual wxSize DoGetBestSize() const override;

    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnMouseDown(wxMouseEvent& evt); // Will primarily handle tab clicks now
    // OnMouseMove and OnMouseLeave might be less relevant here if sub-controls handle their own hover

    FlatUIHomeSpace* GetHomeSpace() { return m_homeSpace; }

private:
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

    // GetTopLevelFrame might now be primarily used by m_systemButtons internally
    // wxFrame* GetTopLevelFrame() const; 
};

#endif // FLATUIBAR_H 