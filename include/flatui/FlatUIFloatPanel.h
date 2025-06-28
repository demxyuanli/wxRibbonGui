#ifndef FLATUIFLOATPANEL_H
#define FLATUIFLOATPANEL_H

#include <wx/wx.h>
#include <wx/timer.h>

class FlatUIPage;
class FlatUIPinButton;

// Custom event for float panel dismissal
wxDECLARE_EVENT(wxEVT_FLOAT_PANEL_DISMISSED, wxCommandEvent);

class FlatUIFloatPanel : public wxFrame
{
public:
    FlatUIFloatPanel(wxWindow* parent);
    virtual ~FlatUIFloatPanel();
    
    // Set the page content to display
    void SetPageContent(FlatUIPage* page);
    
    // Show the float panel at a specified position, with an optional fixed size
    void ShowAt(const wxPoint& position, const wxSize& size = wxDefaultSize);
    
    // Hide and cleanup
    void HidePanel();
    
    // Get current page
    FlatUIPage* GetCurrentPage() const { return m_currentPage; }
    
    // Check if panel should auto-hide based on mouse position
    bool ShouldAutoHide(const wxPoint& globalMousePos) const;
    
    // Force hide the panel (used by parent when needed)
    void ForceHide();

private:
    wxWindow* m_parentWindow;
    wxPanel* m_contentPanel;
    wxBoxSizer* m_sizer;
    FlatUIPage* m_currentPage;
    FlatUIPinButton* m_pinButton;

    // Auto-hide members
    wxTimer m_autoHideTimer;
    static const int AUTO_HIDE_DELAY_MS = 300;
    
    // Appearance members
    wxColour m_borderColour;
    wxColour m_backgroundColour;
    wxColour m_shadowColour;
    int m_borderWidth;
    int m_shadowOffset;
    
    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseEnter(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& event);
    void OnActivate(wxActivateEvent& event);
    void OnKillFocus(wxFocusEvent& event);
    void OnAutoHideTimer(wxTimerEvent& event);
    void OnGlobalMouseMove(wxMouseEvent& event);
    void OnPinButtonClicked(wxCommandEvent& event);
    
    // Auto-hide logic
    void StartAutoHideTimer();
    void StopAutoHideTimer();
    void CheckAutoHide();

    // Layout
    void DoUpdateLayout();
    
    // Helper methods
    void SetupAppearance();
    void SetupEventHandlers();
    void DrawCustomBorder(wxDC& dc);
    void DrawShadow(wxDC& dc);
    
    wxDECLARE_EVENT_TABLE();
};

#endif // FLATUIFLOATPANEL_H 