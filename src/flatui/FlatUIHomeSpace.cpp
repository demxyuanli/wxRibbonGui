#include "flatui/FlatUIHomeSpace.h"
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC
#include <wx/settings.h> // For system colours
#include "flatui/FlatUIConstants.h" // Include for color constants
#include "flatui/FlatUIHomeMenu.h" // For the custom menu
#include "flatui/FlatFrame.h"       // To get parent FlatFrame and content height
#include "flatui/FlatUIBar.h"         // To get FlatUIBar height

// Known menu item IDs from FlatFrame (or define them in a shared constants header)
// These should match the IDs used in FlatFrame's event handlers
// wxID_EXIT is standard

FlatUIHomeSpace::FlatUIHomeSpace(wxWindow* parent, wxWindowID id)
    : wxControl(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE | wxFULL_REPAINT_ON_RESIZE),
      // m_menu(nullptr), // Removed
      m_hover(false),
      m_buttonWidth(DEFAULT_BUTTON_WIDTH),
      m_activeHomeMenu(nullptr) // Initialize m_activeHomeMenu
{
    SetBackgroundStyle(wxBG_STYLE_PAINT); // Important for custom painting

   
    SetMinSize(wxSize(m_buttonWidth, GetParent() ? GetParent()->GetClientSize().GetHeight() : 20)); // Initial min size
}

FlatUIHomeSpace::~FlatUIHomeSpace()
{
    // m_menu is not owned by this class
}

// void FlatUIHomeSpace::SetMenu(wxMenu* menu) { /* m_menu = menu; */ } // Removed

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
    // dc.Clear(); // Clearing with default might not be what we want if aiming for transparency to parent.
                 // Instead, we will explicitly fill our buttonRect with appropriate color.

    CalculateButtonRect(GetClientSize()); 

    wxColour parentBgColor = GetParent() ? GetParent()->GetBackgroundColour() : wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);
    wxColour finalBgColorToDraw;

    if (m_hover) { // m_menu check removed as it's no longer relevant for hover indication
        finalBgColorToDraw = FLATUI_HOMESPACE_HOVER_BG_COLOUR; 
    } else {
        finalBgColorToDraw = parentBgColor; // In normal state, match parent background
    }
    
    // Fill the entire control area with the determined background color
    // This handles the "transparent" background for normal state and hover background
    dc.SetBrush(wxBrush(finalBgColorToDraw));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0, 0, GetClientSize().GetWidth(), GetClientSize().GetHeight()); // Fill entire control

    // Draw icon on top
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

void FlatUIHomeSpace::SetHomeMenu(FlatUIHomeMenu* menu)
{
    m_activeHomeMenu = menu;
}

void FlatUIHomeSpace::OnMouseDown(wxMouseEvent& evt)
{
    if (m_buttonRect.Contains(evt.GetPosition()))
    {
        if (m_activeHomeMenu) { 
            if (m_activeHomeMenu->IsShown()) {
                // m_activeHomeMenu->Close(true); // Close will call Hide and notify us
                m_activeHomeMenu->Hide(); // Simply hide it, OnDismiss or Close will handle notification
                m_hover = false; 
                Refresh();
                // evt.Skip(false); // Usually popups handle mouse events fully
                return; 
            } else {
                // wxWindow* topLevelParent = wxGetTopLevelParent(this);
                // FlatFrame* flatFrame = dynamic_cast<FlatFrame*>(topLevelParent);
                // We don't need flatFrame here anymore as menu size/height calculation might be simpler
                // or done within ShowAt or based on BuildMenuLayout.

                wxPoint menuPos = ClientToScreen(wxPoint(0, m_buttonRect.GetBottom()));
                
                // Determine contentHeight. This might need adjustment.
                // For now, let's use a fixed height or a pre-calculated one.
                // The height could also be determined by the menu itself after BuildMenuLayout.
                int menuContentHeight = 300; // Example fixed height
                // If FlatFrame is needed for height calc:
                // FlatFrame* flatFrame = static_cast<FlatUIHomeMenu*>(m_activeHomeMenu)->GetEventSinkFrame(); // Assuming a getter
                // if(flatFrame) { /* calc height */ }
                                
                m_activeHomeMenu->ShowAt(menuPos, menuContentHeight);
                m_hover = false; 
                Refresh();
                // evt.Skip(false);
                return;
            }
        }
    }
    evt.Skip(); // If click was not on button, skip to allow other processing
}

void FlatUIHomeSpace::OnMouseMove(wxMouseEvent& evt)
{
    bool oldHover = m_hover;
    // Allow hover state regardless of menu, visual feedback for clickability
    m_hover = m_buttonRect.Contains(evt.GetPosition());
   
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

void FlatUIHomeSpace::OnHomeMenuClosed(FlatUIHomeMenu* closedMenu)
{
    if (m_activeHomeMenu == closedMenu) {
        // Menu is already hidden by itself (either via its Close, or OnDismiss)
        m_hover = false; 
        Refresh();       
    }
} 