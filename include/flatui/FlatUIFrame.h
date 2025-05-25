#ifndef FLATUIFRAME_H
#define FLATUIFRAME_H

#include "flatui/BorderlessFrameLogic.h"
#include <wx/log.h>

class FlatUIFrame : public BorderlessFrameLogic
{
public:
    FlatUIFrame(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxBORDER_NONE);
    virtual ~FlatUIFrame();

    // Override mouse events to add specific checks (e.g., pseudo-maximization)
    void OnLeftDown(wxMouseEvent& event) override;
    void OnLeftUp(wxMouseEvent& event) override;
    void OnMotion(wxMouseEvent& event) override;

    // Methods for pseudo-maximization
    bool IsPseudoMaximized() const { return m_isPseudoMaximized; }
    void PseudoMaximize();
    void RestoreFromPseudoMaximize();

    // Generic method to log UI layout structure (remains in this class or a utility class)
    void LogUILayout(wxWindow* window = nullptr, int depth = 0);

protected:
    // Helper method for FlatUIFrame specific style initialization (e.g., background color)
    void InitFrameStyle();

    // Members for pseudo-maximization state
    bool m_isPseudoMaximized;
    wxRect m_preMaximizeRect;            // Stores frame rect before pseudo-maximization

    // Note: Dragging, resizing, rubber band members and methods are now in BorderlessFrameLogic
    // Note: m_borderThreshold is in BorderlessFrameLogic

private:
    // FlatUIFrame may have its own event table if it adds events beyond BorderlessFrameLogic
    // For now, mouse events are routed through BorderlessFrameLogic's table and handled by overrides.
    // If FlatUIFrame had completely different mouse handlers not related to base logic,
    // it would need its own EVT_LEFT_DOWN etc. and its own wxDECLARE_EVENT_TABLE.
    // For now, assume it extends the base logic via overrides.
    // wxDECLARE_EVENT_TABLE(); // Only if FlatUIFrame binds its own, distinct events.
};

#endif // FLATUIFRAME_H 