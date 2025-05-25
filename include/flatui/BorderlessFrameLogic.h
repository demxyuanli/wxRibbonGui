#ifndef BORDERLESSFRAMELOGIC_H
#define BORDERLESSFRAMELOGIC_H

#include <wx/wx.h>

// Enum to represent window edges/corners for resizing
enum class ResizeMode {
    NONE,
    LEFT,
    TOP_LEFT,
    TOP,
    TOP_RIGHT,
    RIGHT,
    BOTTOM_RIGHT,
    BOTTOM,
    BOTTOM_LEFT
};

class BorderlessFrameLogic : public wxFrame
{
public:
    BorderlessFrameLogic(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxBORDER_NONE);
    virtual ~BorderlessFrameLogic();

protected:
    // Core mouse event handlers for dragging and resizing
    virtual void OnLeftDown(wxMouseEvent& event);
    virtual void OnLeftUp(wxMouseEvent& event);
    virtual void OnMotion(wxMouseEvent& event);

    void OnPaint(wxPaintEvent& event);

    // Helper methods for resizing
    ResizeMode GetResizeModeForPosition(const wxPoint& clientPos);
    void UpdateCursorForResizeMode(ResizeMode mode);

    // Rubber band drawing for visual feedback during resize/drag
    void DrawRubberBand(const wxRect& rect);
    void EraseRubberBand();

    // Member variables for custom dragging state
    bool m_dragging;
    wxPoint m_dragStartPos; // Relative to window's top-left for dragging

    // Member variables for custom resizing state
    bool m_resizing;
    ResizeMode m_resizeMode;
    wxPoint m_resizeStartMouseScreenPos; // Initial mouse pos in screen coords for resizing
    wxRect m_resizeStartWindowRect;      // Initial window rect in screen coords for resizing
    wxRect m_currentRubberBandRect;      // Current rect of the rubber band
    bool m_rubberBandVisible;            // Is the rubber band currently visible?
    int m_borderThreshold;               // Pixel threshold to detect border proximity for resizing

private:
    wxDECLARE_EVENT_TABLE();
};

#endif // BORDERLESSFRAMELOGIC_H 