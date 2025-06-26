#ifndef FLATUIFLOATINGWINDOW_H
#define FLATUIFLOATINGWINDOW_H

#include <wx/wx.h>
#include <wx/popupwin.h>

class FlatUIPage;

// Custom event for floating window dismiss
wxDECLARE_EVENT(wxEVT_FLOATING_WINDOW_DISMISSED, wxCommandEvent);

class FlatUIFloatingWindow : public wxPopupTransientWindow
{
public:
    FlatUIFloatingWindow(wxWindow* parent);
    virtual ~FlatUIFloatingWindow();
    
    // Set the page content to display
    void SetPageContent(FlatUIPage* page);
    
    // Show the floating window at a specified position, with an optional fixed size
    void ShowAt(const wxPoint& position, const wxSize& size = wxDefaultSize);
    
    // Hide and cleanup
    void HideWindow();
    
    // Get current page
    FlatUIPage* GetCurrentPage() const { return m_currentPage; }
    
    // Override to handle mouse events properly
    virtual bool ProcessLeftDown(wxMouseEvent& event) override;
    
    // Override to handle auto-dismiss
    virtual void OnDismiss() override;
    
private:
    FlatUIPage* m_currentPage;
    wxBoxSizer* m_sizer;
    
    // Event handlers
    void OnSize(wxSizeEvent& event);
    
    wxDECLARE_EVENT_TABLE();
};

#endif // FLATUIFLOATINGWINDOW_H