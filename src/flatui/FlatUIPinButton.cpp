#include "flatui/FlatUIPinButton.h"
#include "config/SvgIconManager.h"
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)

// Define the custom event
wxDEFINE_EVENT(wxEVT_PIN_BUTTON_CLICKED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(FlatUIPinButton, wxControl)
    EVT_PAINT(FlatUIPinButton::OnPaint)
    EVT_SIZE(FlatUIPinButton::OnSize)
    EVT_MOTION(FlatUIPinButton::OnMouseMove)
    EVT_LEAVE_WINDOW(FlatUIPinButton::OnMouseLeave)
    EVT_LEFT_DOWN(FlatUIPinButton::OnLeftDown)
wxEND_EVENT_TABLE()

FlatUIPinButton::FlatUIPinButton(wxWindow* parent, wxWindowID id,
                                 const wxPoint& pos, const wxSize& size, long style)
    : wxControl(parent, id, pos, size, style | wxBORDER_NONE),
      m_iconHover(false)
{
    SetName("FlatUIPinButton");
    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Set minimum size based on SVG or fallback to default
    wxSize bestSize = DoGetBestSize();
    SetMinSize(bestSize);
    SetSize(bestSize);
    
    LOG_INF("FlatUIPinButton created", "FlatUIPinButton");
}

FlatUIPinButton::~FlatUIPinButton()
{
    LOG_INF("FlatUIPinButton destroyed", "FlatUIPinButton");
}

wxSize FlatUIPinButton::DoGetBestSize() const
{
    try {
        auto& iconManager = SvgIconManager::GetInstance();
        if (iconManager.HasIcon("thumbtack")) {
            // Use a larger fixed size to ensure visibility
            int padding = 4; // 4px padding on each side
            return wxSize(12 + padding * 2, 12 + padding * 2); // 20x20 total
        }
    } catch (...) {
        // Fall through to default size
    }
    
    // Fallback to a larger default size
    return wxSize(20, 20);
}

void FlatUIPinButton::NotifyPinClicked()
{
    // Send custom event to parent
    wxCommandEvent event(wxEVT_PIN_BUTTON_CLICKED, GetId());
    event.SetEventObject(this);
    
    wxWindow* parent = GetParent();
    if (parent) {
        parent->GetEventHandler()->ProcessEvent(event);
    }
}

void FlatUIPinButton::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);

    // Clear background
    dc.SetBackground(wxBrush(GetParent()->GetBackgroundColour()));
    dc.Clear();

    // Draw a visible background for debugging
    dc.SetBrush(wxBrush(wxColour(240, 240, 240)));
    dc.SetPen(wxPen(wxColour(200, 200, 200)));
    dc.DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());

    // Draw hover background
    if (m_iconHover) {
        dc.SetBrush(wxBrush(wxColour(200, 200, 200, 100)));
        dc.SetPen(wxPen(wxColour(150, 150, 150)));
        dc.DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
    }

    // Draw pin icon
    DrawPinIcon(dc);
}

void FlatUIPinButton::OnSize(wxSizeEvent& evt)
{
    Refresh();
    evt.Skip();
}

void FlatUIPinButton::OnMouseMove(wxMouseEvent& evt)
{
    bool wasHover = m_iconHover;
    m_iconHover = true;
    
    if (m_iconHover != wasHover) {
        Refresh();
        SetCursor(wxCursor(wxCURSOR_HAND));
    }
    
    evt.Skip();
}

void FlatUIPinButton::OnMouseLeave(wxMouseEvent& evt)
{
    if (m_iconHover) {
        m_iconHover = false;
        Refresh();
        SetCursor(wxCursor(wxCURSOR_ARROW));
    }
    evt.Skip();
}

void FlatUIPinButton::OnLeftDown(wxMouseEvent& evt)
{
    NotifyPinClicked();
    // Don't skip - we handled this event
}

void FlatUIPinButton::DrawPinIcon(wxDC& dc)
{
    // Try to draw SVG icon first
    bool drewSvg = false;
    
    try {
        DrawSvgIcon(dc);
        drewSvg = true;
    } catch (...) {
        // Fall through to fallback drawing
    }
    
    if (!drewSvg) {
        DrawFallbackIcon(dc);
    }
}

void FlatUIPinButton::DrawSvgIcon(wxDC& dc)
{
    auto& iconManager = SvgIconManager::GetInstance();
    // Use a fixed size to ensure visibility
    wxBitmap iconBitmap = iconManager.GetIconBitmap("thumbtack", wxSize(12, 12));
    
    if (iconBitmap.IsOk()) {
        wxRect clientRect = GetClientRect();
        // Center the icon in the control
        wxPoint iconPos = clientRect.GetPosition() + 
                         wxSize((clientRect.GetWidth() - iconBitmap.GetWidth())/2, 
                               (clientRect.GetHeight() - iconBitmap.GetHeight())/2);
        dc.DrawBitmap(iconBitmap, iconPos);
        LOG_INF("Drew SVG pin icon at (" + std::to_string(iconPos.x) + ", " + std::to_string(iconPos.y) + ")", "FlatUIPinButton");
    } else {
        LOG_INF("SVG icon bitmap is not OK", "FlatUIPinButton");
        throw std::runtime_error("SVG icon not available");
    }
}

void FlatUIPinButton::DrawFallbackIcon(wxDC& dc)
{
    wxRect clientRect = GetClientRect();
    wxPoint center = clientRect.GetPosition() + wxSize(clientRect.GetWidth()/2, clientRect.GetHeight()/2);
    
    // Use a more visible color - dark blue
    wxColour iconColor = wxColour(0, 100, 200);
    dc.SetPen(wxPen(iconColor, 2));
    dc.SetBrush(wxBrush(iconColor));
    
    // Draw pin icon (thumbtack shape) - make it larger
    // Draw the head of the thumbtack
    dc.DrawCircle(center.x, center.y - 3, 4);
    // Draw the pin part
    dc.DrawLine(center.x, center.y + 1, center.x, center.y + 6);
    // Draw the point
    dc.SetPen(wxPen(iconColor, 2));
    dc.DrawLine(center.x - 2, center.y + 6, center.x + 2, center.y + 6);
    
    LOG_INF("Drew fallback pin icon at center (" + std::to_string(center.x) + ", " + std::to_string(center.y) + ")", "FlatUIPinButton");
} 