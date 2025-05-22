#include "flatui/FlatUISystemButtons.h"
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC

FlatUISystemButtons::FlatUISystemButtons(wxWindow* parent, wxWindowID id)
    : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE),
      m_minimizeButtonHover(false),
      m_maximizeButtonHover(false),
      m_closeButtonHover(false),
      m_buttonWidth(DEFAULT_BUTTON_WIDTH),
      m_buttonSpacing(DEFAULT_BUTTON_SPACING)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    SetDoubleBuffered(true);
    
    Bind(wxEVT_PAINT, &FlatUISystemButtons::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FlatUISystemButtons::OnMouseDown, this);
    Bind(wxEVT_MOTION, &FlatUISystemButtons::OnMouseMove, this);
    Bind(wxEVT_LEAVE_WINDOW, &FlatUISystemButtons::OnMouseLeave, this);

    int requiredWidth = 3 * DEFAULT_BUTTON_WIDTH + 2 * DEFAULT_BUTTON_SPACING;
    int buttonHeight = 30; 
    
    SetMinSize(wxSize(requiredWidth, buttonHeight));
    SetSize(GetMinSize());
}

FlatUISystemButtons::~FlatUISystemButtons() {}

int FlatUISystemButtons::GetMyRequiredWidth() const {
    return GetRequiredWidth(m_buttonWidth, m_buttonSpacing);
}

int FlatUISystemButtons::GetRequiredWidth(int buttonWidth, int buttonSpacing) {
    return (buttonWidth * 3) + (buttonSpacing * 2);
}

void FlatUISystemButtons::SetButtonRects(const wxRect& minimizeRect, const wxRect& maximizeRect, const wxRect& closeRect)
{
    // These rects are relative to this FlatUISystemButtons control's own client area.
    // When FlatUIBar calculates their positions, it will be in FlatUIBar's coordinate space.
    // FlatUIBar will then position FlatUISystemButtons control, and then call this method
    // with rects adjusted to be relative to FlatUISystemButtons's origin (0,0).
    
    // For now, assume the rects passed are already relative to this control's (0,0)
    // This means FlatUIBar would calculate absolute positions, then create these rects
    // for us by subtracting our FlatUISystemButtons::GetRect().GetTopLeft().
    // Simpler: FlatUIBar positions us, and we just draw within our GetClientSize().
    // FlatUIBar will tell us our total width/height. We then derive the button rects from that.

    wxSize clientSize = GetClientSize();
    
    int buttonWidth = 40; 
    int buttonHeight = 30;
    
    int sysButtonY = (clientSize.GetHeight() - buttonHeight) / 2;
    if (sysButtonY < 0) sysButtonY = 0;

    // Calculate rects based on our client size, anchored to the right
    int currentX = clientSize.GetWidth();

    currentX -= buttonWidth;
    m_closeButtonRect = wxRect(currentX, sysButtonY, buttonWidth, buttonHeight);
    
    currentX -= m_buttonSpacing;
    currentX -= buttonWidth;
    m_maximizeButtonRect = wxRect(currentX, sysButtonY, buttonWidth, buttonHeight);

    currentX -= m_buttonSpacing;
    currentX -= buttonWidth;
    m_minimizeButtonRect = wxRect(currentX, sysButtonY, buttonWidth, buttonHeight);
    
    // Note: The SetButtonRects in the header was perhaps misleading.
    // This control will draw its buttons within its own bounds, right-aligned.
    // FlatUIBar just needs to give this control the correct overall size and position.
    // So, this method might not be strictly needed if we always calculate from client size in OnPaint
    // Or, it can be used to confirm/override if FlatUIBar wants to dictate sub-rects.
    // For robust encapsulation, this control should manage its internal button layout.
    Refresh(); // In case rects changed affecting hover state display
}

wxFrame* FlatUISystemButtons::GetTopLevelFrame() const
{
    wxWindow* topLevelWindow = wxGetTopLevelParent(const_cast<FlatUISystemButtons*>(this));
    return wxDynamicCast(topLevelWindow, wxFrame);
}

void FlatUISystemButtons::PaintButton(wxDC& dc, const wxRect& rect, const wxString& symbol, bool hover, bool isClose, bool isMaximized)
{
    wxColour btnTextColour = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    wxColour hoverBgColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    wxColour hoverTextColour = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    wxColour normalBgColour = GetParent() ? GetParent()->GetBackgroundColour() : wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR); // Inherit from parent (FlatUIBar)

    if (isClose) {
        dc.SetBrush(hover ? wxColour(232, 17, 35) : normalBgColour);
        dc.SetPen(hover ? wxColour(232, 17, 35) : *wxGREY_PEN);
        dc.SetTextForeground(hover ? *wxWHITE : btnTextColour);
    }
    else {
        dc.SetBrush(hover ? hoverBgColour : normalBgColour);
        dc.SetPen(*wxGREY_PEN);
        dc.SetTextForeground(hover ? hoverTextColour : btnTextColour);
    }
    dc.DrawRectangle(rect);
    
    wxSize textSize = dc.GetTextExtent(symbol);
    int textY = rect.GetY() + (rect.GetHeight() - textSize.GetHeight()) / 2;
    if (symbol == "_") textY = rect.GetY() + (rect.GetHeight() - textSize.GetHeight()) / 4; // Lower minimize slightly

    dc.DrawText(symbol, rect.GetX() + (rect.GetWidth() - textSize.GetWidth()) / 2, textY);
}

void FlatUISystemButtons::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear(); // Clear our own background
    
    // Recalculate button rects based on current client size every paint, ensures responsiveness
    // This makes SetButtonRects(ext, ext, ext) less critical for passing specific sub-rects
    // and more about signaling that FlatUIBar has updated our overall size.
    wxSize currentSize = GetClientSize();
    
    int currentButtonHeight = 30;
    int currentButtonWidth = 40;
    
    int currentButtonY = (currentSize.GetHeight() - currentButtonHeight) / 2;
    if (currentButtonY < 0) currentButtonY = 0;
    
    int xPos = currentSize.GetWidth(); // Start from the right edge
    
    xPos -= currentButtonWidth;
    m_closeButtonRect = wxRect(xPos, currentButtonY, currentButtonWidth, currentButtonHeight);
    
    xPos -= m_buttonSpacing;
    xPos -= currentButtonWidth;
    m_maximizeButtonRect = wxRect(xPos, currentButtonY, currentButtonWidth, currentButtonHeight);
    
    xPos -= m_buttonSpacing;
    xPos -= currentButtonWidth;
    m_minimizeButtonRect = wxRect(xPos, currentButtonY, currentButtonWidth, currentButtonHeight);

    wxFrame* topFrame = GetTopLevelFrame();
    bool isMaximized = topFrame ? topFrame->IsMaximized() : false;

    PaintButton(dc, m_minimizeButtonRect, "_", m_minimizeButtonHover);
    PaintButton(dc, m_maximizeButtonRect, isMaximized ? wxString(wchar_t(0x2750)) : wxString(wchar_t(0x2610)), m_maximizeButtonHover, false, isMaximized);
    PaintButton(dc, m_closeButtonRect, "X", m_closeButtonHover, true);
}

void FlatUISystemButtons::OnMouseDown(wxMouseEvent& evt)
{
    HandleSystemButtonAction(evt.GetPosition(), evt); 
    // evt.Skip() is handled in HandleSystemButtonAction
}

void FlatUISystemButtons::HandleSystemButtonAction(const wxPoint& pos, wxMouseEvent& evt) 
{
    wxFrame* frame = GetTopLevelFrame();
    if (!frame) { evt.Skip(); return; }

    bool handled = false;
    if (m_closeButtonRect.Contains(pos)) {
        frame->Close(true);
        handled = true;
    }
    else if (m_maximizeButtonRect.Contains(pos)) {
        if (frame->IsMaximized())
            frame->Restore();
        else
            frame->Maximize();
        handled = true;
    }
    else if (m_minimizeButtonRect.Contains(pos)) {
        frame->Iconize(true);
        handled = true;
    }
    
    if (handled) {
        SetFocus(); // Keep focus or manage as needed
        evt.Skip(false); // Event consumed
    }
    else {
        evt.Skip(); // Event not for us
    }
}

void FlatUISystemButtons::OnMouseMove(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    bool needsRefresh = false;

    bool oldHoverMin = m_minimizeButtonHover;
    m_minimizeButtonHover = m_minimizeButtonRect.Contains(pos);
    if (oldHoverMin != m_minimizeButtonHover) needsRefresh = true;

    bool oldHoverMax = m_maximizeButtonHover;
    m_maximizeButtonHover = m_maximizeButtonRect.Contains(pos);
    if (oldHoverMax != m_maximizeButtonHover) needsRefresh = true;

    bool oldHoverClose = m_closeButtonHover;
    m_closeButtonHover = m_closeButtonRect.Contains(pos);
    if (oldHoverClose != m_closeButtonHover) needsRefresh = true;

    if (needsRefresh) {
        Refresh();
    }
    evt.Skip(); 
}

void FlatUISystemButtons::OnMouseLeave(wxMouseEvent& evt)
{
    bool needsRefresh = false;
    if (m_minimizeButtonHover) { m_minimizeButtonHover = false; needsRefresh = true; }
    if (m_maximizeButtonHover) { m_maximizeButtonHover = false; needsRefresh = true; }
    if (m_closeButtonHover) { m_closeButtonHover = false; needsRefresh = true; }

    if (needsRefresh) {
        Refresh();
    }
    evt.Skip();
} 