#include "flatui/FlatUIPinControl.h"
#include "config/SvgIconManager.h"
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)

// Define the custom event
wxDEFINE_EVENT(wxEVT_PIN_STATE_CHANGED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(FlatUIPinControl, wxControl)
    EVT_PAINT(FlatUIPinControl::OnPaint)
    EVT_SIZE(FlatUIPinControl::OnSize)
    EVT_MOTION(FlatUIPinControl::OnMouseMove)
    EVT_LEAVE_WINDOW(FlatUIPinControl::OnMouseLeave)
    EVT_LEFT_DOWN(FlatUIPinControl::OnLeftDown)
wxEND_EVENT_TABLE()

FlatUIPinControl::FlatUIPinControl(wxWindow* parent, wxWindowID id,
                                   const wxPoint& pos, const wxSize& size, long style)
    : wxControl(parent, id, pos, size, style | wxBORDER_NONE),
      m_isPinned(false),
      m_iconHover(false)
{
    SetName("FlatUIPinControl");
    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Set minimum size
    SetMinSize(wxSize(CONTROL_WIDTH, CONTROL_HEIGHT));
    SetSize(wxSize(CONTROL_WIDTH, CONTROL_HEIGHT));
    
    LOG_INF("FlatUIPinControl created", "FlatUIPinControl");
}

FlatUIPinControl::~FlatUIPinControl()
{
    LOG_INF("FlatUIPinControl destroyed", "FlatUIPinControl");
}

wxSize FlatUIPinControl::DoGetBestSize() const
{
    return wxSize(CONTROL_WIDTH, CONTROL_HEIGHT);
}

void FlatUIPinControl::SetPinned(bool pinned)
{
    if (m_isPinned != pinned) {
        m_isPinned = pinned;
        NotifyPinStateChanged();
        Refresh();
        
        LOG_INF("Pin state changed to: " + std::string(pinned ? "pinned" : "unpinned"), 
                "FlatUIPinControl");
    }
}

void FlatUIPinControl::TogglePinState()
{
    SetPinned(!m_isPinned);
}

void FlatUIPinControl::NotifyPinStateChanged()
{
    // Send custom event to parent
    wxCommandEvent event(wxEVT_PIN_STATE_CHANGED, GetId());
    event.SetEventObject(this);
    event.SetInt(m_isPinned ? 1 : 0);
    
    wxWindow* parent = GetParent();
    if (parent) {
        parent->GetEventHandler()->ProcessEvent(event);
    }
}

void FlatUIPinControl::OnPaint(wxPaintEvent& evt)
{
    LOG_INF("Pin control OnPaint called", "FlatUIPinControl");

    wxAutoBufferedPaintDC dc(this);

    // Clear background
    dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
    dc.Clear();

    LOG_INF("  Control size: " + std::to_string(GetSize().GetWidth()) + "x" + std::to_string(GetSize().GetHeight()), "FlatUIPinControl");
    LOG_INF("  Control position: (" + std::to_string(GetPosition().x) + ", " + std::to_string(GetPosition().y) + ")", "FlatUIPinControl");
    LOG_INF("  Control visible: " + std::string(IsShown() ? "true" : "false"), "FlatUIPinControl");

    // ... existing code ...
    // Draw hover background
    if (m_iconHover) {
        dc.SetBrush(wxBrush(wxColour(200, 200, 200, 50)));
        dc.SetPen(wxPen(wxColour(200, 200, 200, 50)));
        dc.DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
    }

    // Draw pin icon
    DrawPinIcon(dc);

    LOG_INF("Pin control paint completed", "FlatUIPinControl");
}

void FlatUIPinControl::OnSize(wxSizeEvent& evt)
{
    Refresh();
    evt.Skip();
}

void FlatUIPinControl::OnMouseMove(wxMouseEvent& evt)
{
    bool wasHover = m_iconHover;
    m_iconHover = true;
    
    if (m_iconHover != wasHover) {
        Refresh();
        SetCursor(wxCursor(wxCURSOR_HAND));
    }
    
    evt.Skip();
}

void FlatUIPinControl::OnMouseLeave(wxMouseEvent& evt)
{
    if (m_iconHover) {
        m_iconHover = false;
        Refresh();
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    evt.Skip();
}

void FlatUIPinControl::OnLeftDown(wxMouseEvent& evt)
{
    TogglePinState();
    // Don't skip - we handled this event
}

void FlatUIPinControl::DrawPinIcon(wxDC& dc)
{
    wxRect clientRect = GetClientRect();
    wxPoint center = clientRect.GetPosition() + wxSize(clientRect.GetWidth()/2, clientRect.GetHeight()/2);
    
    // Try to draw SVG icon first
    bool drewSvg = false;
    
    try {
        auto& iconManager = SvgIconManager::GetInstance();
        wxString iconName = m_isPinned ? "unpin" : "thumbtack";
        
        if (iconManager.HasIcon(iconName)) {
            DrawSvgIcon(dc, iconName);
            drewSvg = true;
        }
    } catch (...) {
        // Fall through to fallback drawing
    }
    
    if (!drewSvg) {
        DrawFallbackIcon(dc);
    }
}

void FlatUIPinControl::DrawSvgIcon(wxDC& dc, const wxString& iconName)
{
    auto& iconManager = SvgIconManager::GetInstance();
    wxBitmap iconBitmap = iconManager.GetIconBitmap(iconName, wxSize(16,16));
    
    if (iconBitmap.IsOk()) {
        wxRect clientRect = GetClientRect();
        wxPoint iconPos = clientRect.GetPosition() + 
                         wxSize((clientRect.GetWidth() - ICON_SIZE)/2, 
                               (clientRect.GetHeight() - ICON_SIZE)/2);
        dc.DrawBitmap(iconBitmap, iconPos);
    }
}

void FlatUIPinControl::DrawFallbackIcon(wxDC& dc)
{
    wxRect clientRect = GetClientRect();
    wxPoint center = clientRect.GetPosition() + wxSize(clientRect.GetWidth()/2, clientRect.GetHeight()/2);
    
    dc.SetPen(wxPen(CFG_COLOUR("BarInactiveTextColour"), 1));
    dc.SetBrush(wxBrush(CFG_COLOUR("BarInactiveTextColour")));
    
    if (m_isPinned) {
        // Draw undo arrow (curved arrow)
        dc.DrawCircle(center, 2);
        dc.DrawLine(center.x - 2, center.y - 2, center.x + 1, center.y - 2);
        dc.DrawLine(center.x + 1, center.y - 2, center.x, center.y - 3);
    } else {
        // Draw thumbtack
        dc.DrawCircle(center, 1);
        dc.DrawLine(center.x, center.y + 1, center.x, center.y + 3);
    }
}