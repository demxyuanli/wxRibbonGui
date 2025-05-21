#include "flatui/FlatUIHomeSpace.h"
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC
#include <wx/settings.h> // For system colours

FlatUIHomeSpace::FlatUIHomeSpace(wxWindow* parent, wxWindowID id)
    : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE),
      m_menu(nullptr),
      m_hover(false),
      m_buttonWidth(DEFAULT_BUTTON_WIDTH) // Initialize with default width
{
    SetBackgroundStyle(wxBG_STYLE_PAINT); // Important for custom painting

   
    SetMinSize(wxSize(m_buttonWidth, GetParent() ? GetParent()->GetClientSize().GetHeight() : 20)); // Initial min size
}

FlatUIHomeSpace::~FlatUIHomeSpace()
{
    // m_menu is not owned by this class
}

void FlatUIHomeSpace::SetMenu(wxMenu* menu)
{
    m_menu = menu;
}

void FlatUIHomeSpace::SetIcon(const wxBitmap& icon)
{
    m_icon = icon;
    Refresh();
}

// SetButtonWidth and GetButtonWidth are inline in the header

void FlatUIHomeSpace::CalculateButtonRect(const wxSize& controlSize)
{
    // The button occupies the entire control area for FlatUIHomeSpace
    m_buttonRect = wxRect(0, 0, controlSize.GetWidth(), controlSize.GetHeight());
}

void FlatUIHomeSpace::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear(); // Clear background

    CalculateButtonRect(GetClientSize()); // Ensure rect is up-to-date

    wxColour bgColor = GetParent() ? GetParent()->GetBackgroundColour() : wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);
    if (m_hover && m_menu) { // Only show hover if there's a menu to interact with
        // Manually darken the color
        int red = bgColor.Red() * 0.8;
        int green = bgColor.Green() * 0.8;
        int blue = bgColor.Blue() * 0.8;
        bgColor.Set(red, green, blue);
    }
    dc.SetBrush(bgColor);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(m_buttonRect);

    if (m_icon.IsOk())
    {
        int x = (m_buttonRect.GetWidth() - m_icon.GetWidth()) / 2;
        int y = (m_buttonRect.GetHeight() - m_icon.GetHeight()) / 2;
        dc.DrawBitmap(m_icon, x, y, true /* use mask */);
    }
    else
    {
        // Draw default "hamburger" icon if no icon is set
        int lineCount = 3;
        int VMargin = m_buttonRect.GetHeight() / 4;
        int HMargin = m_buttonRect.GetWidth() / 5;
        int lineThickness = 2;
        int lineSpacing = (m_buttonRect.GetHeight() - 2 * VMargin - lineCount * lineThickness) / wxMax(1, lineCount -1);
        
        dc.SetPen(wxPen(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT), lineThickness));
        for (int i = 0; i < lineCount; ++i) {
            int yPos = VMargin + i * (lineThickness + lineSpacing);
            dc.DrawLine(m_buttonRect.GetLeft() + HMargin, 
                        m_buttonRect.GetTop() + yPos,
                        m_buttonRect.GetRight() - HMargin,
                        m_buttonRect.GetTop() + yPos);
        }
    }
}

void FlatUIHomeSpace::OnMouseDown(wxMouseEvent& evt)
{
    if (m_buttonRect.Contains(evt.GetPosition()))
    {
        if (m_menu)
        {
            wxPoint buttonPos(0, m_buttonRect.GetHeight());

            PopupMenu(m_menu, buttonPos);

            m_hover = false; // Reset hover after click
            Refresh();
            evt.Skip(false); // Consumed
            return;
        }
    }
    evt.Skip();
}

void FlatUIHomeSpace::OnMouseMove(wxMouseEvent& evt)
{
    bool oldHover = m_hover;
    if (m_menu) { // Only allow hover state if there is a menu
         m_hover = m_buttonRect.Contains(evt.GetPosition());
    } else {
        m_hover = false;
    }
   
    if (m_hover != oldHover) {
        Refresh();
    }
    evt.Skip();
}

void FlatUIHomeSpace::OnMouseLeave(wxMouseEvent& evt)
{
    if (m_hover) {
        m_hover = false;
        Refresh();
    }
    evt.Skip();
} 