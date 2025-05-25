#include "flatui/BorderlessFrameLogic.h"
#include <wx/dcbuffer.h> // For wxScreenDC if used, and double buffering

#ifdef __WXMSW__
#include <windows.h> // For Windows specific GDI calls for rubber band
#endif
#include <flatui/FlatUIConstants.h>

wxBEGIN_EVENT_TABLE(BorderlessFrameLogic, wxFrame)
    EVT_LEFT_DOWN(BorderlessFrameLogic::OnLeftDown)
    EVT_LEFT_UP(BorderlessFrameLogic::OnLeftUp)
    EVT_MOTION(BorderlessFrameLogic::OnMotion)
    EVT_PAINT(BorderlessFrameLogic::OnPaint)
wxEND_EVENT_TABLE()

BorderlessFrameLogic::BorderlessFrameLogic(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style),
      m_dragging(false),
      m_resizing(false),
      m_resizeMode(ResizeMode::NONE),
      m_rubberBandVisible(false),
      m_borderThreshold(8) // Default border threshold
{
    // Basic setup, often common for borderless windows
    SetDoubleBuffered(true);
}

BorderlessFrameLogic::~BorderlessFrameLogic()
{
}

void BorderlessFrameLogic::OnLeftDown(wxMouseEvent& event)
{
    // Basic drag/resize initiation logic (without pseudo-maximization check, which is for derived class)
    // Derived classes (like FlatUIFrame) will call this and add their specific checks first.

    // Check if the event originated from a child control that should handle it.
    // This is a placeholder; derived classes should implement more specific logic
    // if they have interactive child controls in draggable/resizable areas.
    wxWindow* eventSource = dynamic_cast<wxWindow*>(event.GetEventObject());
    if (eventSource != this && eventSource->GetParent() == this) { // Example: direct child
        // If child wants to handle it, it should not Skip() or this logic takes over.
        // This area needs careful handling in a complex UI.
    }

    ResizeMode hoverMode = GetResizeModeForPosition(event.GetPosition());
    if (hoverMode != ResizeMode::NONE)
    {
        m_resizing = true;
        m_resizeMode = hoverMode;
        m_resizeStartMouseScreenPos = wxGetMousePosition();
        m_resizeStartWindowRect = GetScreenRect();
        if (!HasCapture()) {
            CaptureMouse();
        }
    }
    else // Click was on the main frame body (not border), initiate drag
    {
        m_dragging = true;
        // m_dragStartPos is mouse position relative to the window's client area top-left
        m_dragStartPos = event.GetPosition(); 
        m_resizeStartWindowRect = GetScreenRect(); // Save initial window rect for rubber band reference
        if (!HasCapture()) {
            CaptureMouse();
        }
    }
    event.Skip(); // Allow further processing if needed by wxWidgets or other handlers
}

void BorderlessFrameLogic::OnLeftUp(wxMouseEvent& event)
{
    if (HasCapture()) {
        ReleaseMouse();
    }

    if (m_dragging) {
        if (m_rubberBandVisible) EraseRubberBand();
        wxPoint mousePosOnScreen = wxGetMousePosition();
        // New window position is current mouse position on screen minus the initial offset (m_dragStartPos)
        int newX = mousePosOnScreen.x - m_dragStartPos.x;
        int newY = mousePosOnScreen.y - m_dragStartPos.y;
        Move(newX, newY);
        m_dragging = false;
    }
    else if (m_resizing) {
        if (m_rubberBandVisible) EraseRubberBand();
        wxPoint currentMouseScreenPos = wxGetMousePosition();
        int dx = currentMouseScreenPos.x - m_resizeStartMouseScreenPos.x;
        int dy = currentMouseScreenPos.y - m_resizeStartMouseScreenPos.y;
        wxRect newRect = m_resizeStartWindowRect;
        int minWidth = GetMinWidth() > 0 ? GetMinWidth() : 100; // Basic min size
        int minHeight = GetMinHeight() > 0 ? GetMinHeight() : 100;

        switch (m_resizeMode)
        {
        case ResizeMode::LEFT:
            newRect.x += dx;
            newRect.width -= dx;
            if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
            break;
        case ResizeMode::RIGHT:
            newRect.width += dx;
            if (newRect.width < minWidth) newRect.width = minWidth;
            break;
        case ResizeMode::TOP:
            newRect.y += dy;
            newRect.height -= dy;
            if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
            break;
        case ResizeMode::BOTTOM:
            newRect.height += dy;
            if (newRect.height < minHeight) newRect.height = minHeight;
            break;
        case ResizeMode::TOP_LEFT:
            newRect.x += dx; newRect.width -= dx;
            newRect.y += dy; newRect.height -= dy;
            if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
            if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
            break;
        case ResizeMode::TOP_RIGHT:
            newRect.width += dx;
            newRect.y += dy; newRect.height -= dy;
            if (newRect.width < minWidth) newRect.width = minWidth;
            if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
            break;
        case ResizeMode::BOTTOM_LEFT:
            newRect.x += dx; newRect.width -= dx;
            newRect.height += dy;
            if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
            if (newRect.height < minHeight) newRect.height = minHeight;
            break;
        case ResizeMode::BOTTOM_RIGHT:
            newRect.width += dx; newRect.height += dy;
            if (newRect.width < minWidth) newRect.width = minWidth;
            if (newRect.height < minHeight) newRect.height = minHeight;
            break;
        case ResizeMode::NONE: break; // Should not happen if m_resizing is true
        }
        SetSize(newRect); // Apply the new size and position
        Layout(); // Recalculate layout if sizers are used
        Refresh(); // Redraw the window and its children
        Update();  // Ensure UI updates are processed

        m_resizing = false;
        m_resizeMode = ResizeMode::NONE;
        UpdateCursorForResizeMode(ResizeMode::NONE); // Reset cursor to arrow
    }
    event.Skip();
}

void BorderlessFrameLogic::OnMotion(wxMouseEvent& event)
{
    // Basic rubber band and cursor update logic (without pseudo-maximization check)
    // Derived classes (like FlatUIFrame) will call this and add their specific checks first.

    if (m_dragging && event.Dragging() && event.LeftIsDown()) {
        if (m_rubberBandVisible) EraseRubberBand();
        wxPoint mousePosOnScreen = wxGetMousePosition();
        wxRect newRect(
            mousePosOnScreen.x - m_dragStartPos.x, // Top-left X of dragging rect
            mousePosOnScreen.y - m_dragStartPos.y, // Top-left Y of dragging rect
            m_resizeStartWindowRect.GetWidth(),    // Original width
            m_resizeStartWindowRect.GetHeight()    // Original height
        );
        DrawRubberBand(newRect);
    }
    else if (m_resizing && event.Dragging() && event.LeftIsDown()) {
        if (m_rubberBandVisible) EraseRubberBand();
        wxPoint currentMouseScreenPos = wxGetMousePosition();
        int dx = currentMouseScreenPos.x - m_resizeStartMouseScreenPos.x;
        int dy = currentMouseScreenPos.y - m_resizeStartMouseScreenPos.y;
        wxRect newRect = m_resizeStartWindowRect;
        int minWidth = GetMinWidth() > 0 ? GetMinWidth() : 100;
        int minHeight = GetMinHeight() > 0 ? GetMinHeight() : 100;
        
        // Calculate newRect based on resizeMode, dx, dy, and minWidth/minHeight
        // (This logic is identical to OnLeftUp's resizing part for drawing rubber band)
        switch (m_resizeMode)
        {
        case ResizeMode::LEFT:
            newRect.x += dx;
            newRect.width -= dx;
            if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
            break;
        case ResizeMode::RIGHT:
            newRect.width += dx;
            if (newRect.width < minWidth) newRect.width = minWidth;
            break;
        case ResizeMode::TOP:
            newRect.y += dy; newRect.height -= dy;
            if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
            break;
        case ResizeMode::BOTTOM:
            newRect.height += dy;
            if (newRect.height < minHeight) newRect.height = minHeight;
            break;
        case ResizeMode::TOP_LEFT:
            newRect.x += dx; newRect.width -= dx;
            newRect.y += dy; newRect.height -= dy;
            if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
            if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
            break;
        case ResizeMode::TOP_RIGHT:
            newRect.width += dx;
            newRect.y += dy; newRect.height -= dy;
            if (newRect.width < minWidth) newRect.width = minWidth;
            if (newRect.height < minHeight) { newRect.height = minHeight; newRect.y = m_resizeStartWindowRect.GetBottom() - minHeight; }
            break;
        case ResizeMode::BOTTOM_LEFT:
            newRect.x += dx; newRect.width -= dx;
            newRect.height += dy;
            if (newRect.width < minWidth) { newRect.width = minWidth; newRect.x = m_resizeStartWindowRect.GetRight() - minWidth; }
            if (newRect.height < minHeight) newRect.height = minHeight;
            break;
        case ResizeMode::BOTTOM_RIGHT:
            newRect.width += dx; newRect.height += dy;
            if (newRect.width < minWidth) newRect.width = minWidth;
            if (newRect.height < minHeight) newRect.height = minHeight;
            break;
        case ResizeMode::NONE: break;
        }
        DrawRubberBand(newRect);
    }
    else if (!event.LeftIsDown()) { // Only update cursor if mouse button is not down
        ResizeMode hoverMode = GetResizeModeForPosition(event.GetPosition());
        UpdateCursorForResizeMode(hoverMode);
    }
    event.Skip();
}

ResizeMode BorderlessFrameLogic::GetResizeModeForPosition(const wxPoint& clientPos)
{
    wxSize clientSize = GetClientSize();
    int x = clientPos.x;
    int y = clientPos.y;
    bool onLeft = (x >= 0 && x < m_borderThreshold);
    bool onRight = (x >= clientSize.GetWidth() - m_borderThreshold && x < clientSize.GetWidth());
    bool onTop = (y >= 0 && y < m_borderThreshold);
    bool onBottom = (y >= clientSize.GetHeight() - m_borderThreshold && y < clientSize.GetHeight());

    if (onTop && onLeft) return ResizeMode::TOP_LEFT;
    if (onBottom && onLeft) return ResizeMode::BOTTOM_LEFT;
    if (onTop && onRight) return ResizeMode::TOP_RIGHT;
    if (onBottom && onRight) return ResizeMode::BOTTOM_RIGHT;
    if (onLeft) return ResizeMode::LEFT;
    if (onRight) return ResizeMode::RIGHT;
    if (onTop) return ResizeMode::TOP;
    if (onBottom) return ResizeMode::BOTTOM;
    return ResizeMode::NONE;
}

void BorderlessFrameLogic::UpdateCursorForResizeMode(ResizeMode mode)
{
    wxStockCursor cursorId = wxCURSOR_ARROW;
    switch (mode)
    {
    case ResizeMode::LEFT: case ResizeMode::RIGHT: cursorId = wxCURSOR_SIZEWE; break;
    case ResizeMode::TOP: case ResizeMode::BOTTOM: cursorId = wxCURSOR_SIZENS; break;
    case ResizeMode::TOP_LEFT: case ResizeMode::BOTTOM_RIGHT: cursorId = wxCURSOR_SIZENWSE; break;
    case ResizeMode::TOP_RIGHT: case ResizeMode::BOTTOM_LEFT: cursorId = wxCURSOR_SIZENESW; break;
    case ResizeMode::NONE: default: cursorId = wxCURSOR_ARROW; break;
    }
    SetCursor(wxCursor(cursorId));
}

void BorderlessFrameLogic::DrawRubberBand(const wxRect& rect)
{
#ifdef __WXMSW__
    HDC hdc = ::GetDC(NULL); 
    RECT winRect;
    winRect.left = rect.GetLeft();
    winRect.top = rect.GetTop();
    winRect.right = rect.GetRight() + 1; 
    winRect.bottom = rect.GetBottom() + 1;

    int oldROP = ::SetROP2(hdc, R2_XORPEN); 

    HPEN hPen = ::CreatePen(PS_DOT, 1, RGB(0, 0, 0)); 
    HPEN hOldPen = (HPEN)::SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)::SelectObject(hdc, GetStockObject(NULL_BRUSH)); 

    ::Rectangle(hdc, winRect.left, winRect.top, winRect.right, winRect.bottom);

    ::SelectObject(hdc, hOldPen);
    ::SelectObject(hdc, hOldBrush);
    ::DeleteObject(hPen);
    ::SetROP2(hdc, oldROP);
    ::ReleaseDC(NULL, hdc);

    m_rubberBandVisible = true;
    m_currentRubberBandRect = rect;
#else
    wxScreenDC dc;
    dc.SetLogicalFunction(wxINVERT);
    dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_DOT));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(rect);

    m_rubberBandVisible = true;
    m_currentRubberBandRect = rect;
#endif
}

void BorderlessFrameLogic::EraseRubberBand()
{
    if (m_rubberBandVisible)
    {
        DrawRubberBand(m_currentRubberBandRect); 
        m_rubberBandVisible = false;
    }
} 

void BorderlessFrameLogic::OnPaint(wxPaintEvent& event)
{
    wxPaintDC dc(this);
    wxSize sz = GetClientSize();
    dc.SetPen(wxPen(*wxRED, 1));
    dc.DrawLine(0, 0, sz.x, 0);           
    dc.DrawLine(0, sz.y - 1, sz.x, sz.y - 1); 
    dc.DrawLine(0, 0, 0, sz.y);           
    dc.DrawLine(sz.x - 1, 0, sz.x - 1, sz.y); 
    event.Skip(); 
}