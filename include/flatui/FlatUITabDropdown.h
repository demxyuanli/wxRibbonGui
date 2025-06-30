#ifndef FLATUI_TAB_DROPDOWN_H
#define FLATUI_TAB_DROPDOWN_H

#include <wx/wx.h>
#include <vector>

// Forward declarations
class FlatUIPage;
class FlatUIBar;

class FlatUITabDropdown : public wxControl
{
public:
    FlatUITabDropdown(wxWindow* parent, wxWindowID id = wxID_ANY,
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxDefaultSize,
                     long style = 0);
    virtual ~FlatUITabDropdown();

    // Menu management
    void UpdateHiddenTabs(const std::vector<size_t>& hiddenIndices);
    void ClearMenu();
    
    // Visibility control
    void ShowDropdown(bool show = true);
    void HideDropdown() { ShowDropdown(false); }
    bool IsDropdownShown() const;
    
    // Position and size
    void SetDropdownRect(const wxRect& rect);
    wxRect GetDropdownRect() const { return m_dropdownRect; }
    
    // Parent bar reference
    void SetParentBar(FlatUIBar* parentBar) { m_parentBar = parentBar; }
    FlatUIBar* GetParentBar() const { return m_parentBar; }
    
    // Menu operations
    void ShowMenu();

    // Override for best size calculation
    virtual wxSize DoGetBestSize() const override;

protected:
    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnMenuItemSelected(wxCommandEvent& event);

private:
    // Drawing
    void DrawDropdownButton(wxDC& dc);
    void DrawDropdownArrow(wxDC& dc, const wxRect& rect);
    
    // Menu management
    void CreateMenu();
    void PopulateMenu();
    
    // State management
    bool IsMouseOver() const { return m_isMouseOver; }
    void SetMouseOver(bool over);

    // Member variables
    FlatUIBar* m_parentBar;
    wxMenu* m_dropdownMenu;
    std::vector<size_t> m_hiddenTabIndices;
    wxRect m_dropdownRect;
    bool m_isMouseOver;
    bool m_isPressed;
    
    // Menu ID range for hidden tabs
    static constexpr int MENU_ID_START = 5000;
    static constexpr int MENU_ID_END = 5999;

    wxDECLARE_EVENT_TABLE();
};

#endif // FLATUI_TAB_DROPDOWN_H 