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
    static const int FLATUI_BAR_HEIGHT = 30;
    
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
    
    void SetTabFunctionSpacer(int width, bool drawSeparator = false);
    void SetFunctionProfileSpacer(int width, bool drawSeparator = false);
    
    void SetTabFunctionSpacerAutoExpand(bool autoExpand);
    
    void SetFunctionProfileSpacerAutoExpand(bool autoExpand);

    static int GetBarHeight(); // Renamed from GetTabAreaHeight for clarity

    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);
    void OnMouseDown(wxMouseEvent& evt); // Will primarily handle tab clicks now
    // OnMouseMove and OnMouseLeave might be less relevant here if sub-controls handle their own hover

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
    static const int TAB_PADDING = 10; 
    static const int TAB_SPACING = 1;
    
    // --- General Layout Constants (can be adjusted) ---
    static const int ELEMENT_SPACING = 5; // General spacing between different major sections
    static const int BAR_PADDING = 2;     // Padding at the very left/right of the bar itself

    // --- Helper methods ---
    void UpdateElementPositionsAndSizes(const wxSize& barSize);
    void PaintTabs(wxDC& dc, int availableWidth, int& currentXOffset); // For drawing tabs directly
    int CalculateTabsWidth(wxDC& dc) const; // For calculating width needed by direct tabs

    // GetTopLevelFrame might now be primarily used by m_systemButtons internally
    // wxFrame* GetTopLevelFrame() const; 
};

#endif // FLATUIBAR_H 