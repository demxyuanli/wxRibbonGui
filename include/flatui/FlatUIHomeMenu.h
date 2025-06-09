#ifndef FLATUIHOMEMENU_H
#define FLATUIHOMEMENU_H

#include <wx/wx.h>
// #include <wx/frame.h> // No longer a wxFrame
#include <wx/popupwin.h> // New base class
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/vector.h>
#include <wx/settings.h> // For system colours and metrics

// Forward declare the main frame if events are sent back
class FlatUIFrame;
class FlatUIHomeSpace; // Forward declare for OnHomeMenuClosed in derived class if needed by base

// Structure for menu items
struct FlatHomeMenuItemInfo {
    int id;
    wxString text;
    wxBitmap icon;
    wxRect rect; // For hit-testing if drawn manually
    bool isSeparator;

    FlatHomeMenuItemInfo(const wxString& txt = wxEmptyString, int itemId = wxID_ANY, const wxBitmap& bmp = wxNullBitmap, bool sep = false)
        : id(itemId), text(txt), icon(bmp), isSeparator(sep) {}
};

class FlatUIHomeMenu : public wxPopupTransientWindow
{
public:
    // Constructor now takes its wxWindow parent and the FlatFrame for event sinking
    FlatUIHomeMenu(wxWindow* parent, FlatUIFrame* eventSinkFrame);
    virtual ~FlatUIHomeMenu();

    void AddMenuItem(const wxString& text, int id, const wxBitmap& icon = wxNullBitmap);
    void AddSeparator();
    void BuildMenuLayout(); // To add items to the sizer

    bool ProcessEvent(wxEvent& event);
    
    // Controls showing the popup at a specific position and size
    void ShowAt(const wxPoint& pos, int contentHeight, bool& isShow);
    virtual bool Close(bool force = true); // Keep for consistency, will mostly call Hide()

    FlatUIFrame* GetEventSinkFrame() const { return m_eventSinkFrame; } // Getter for event sink frame

protected:
    void OnPaint(wxPaintEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnActivate(wxActivateEvent& event);
    void OnMouseMotion(wxMouseEvent& event); // For hover effects on items
    void OnDismiss(); // wxPopupWindow specific for when it's dismissed
    void OnMouseClickOutside(wxMouseEvent& event);

private:
    FlatUIFrame* m_eventSinkFrame; // To send events to (renamed from m_parentFrame for clarity)
    // m_ownerWindow is no longer needed as GetParent() from wxPopupWindow gives the wx parent
    wxPanel* m_panel;         // Main panel for content
    wxBoxSizer* m_itemSizer;  // Sizer for menu items

    wxVector<FlatHomeMenuItemInfo> m_menuItems;
    
    void SendItemCommand(int id);

wxDECLARE_EVENT_TABLE();
};

#endif // FLATUIHOMEMENU_H 