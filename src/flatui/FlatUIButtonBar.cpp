#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIConstants.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIEventManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>

// Calculate this one if needed, or ensure it's derived correctly where used
constexpr int FLATUI_BUTTONBAR_INTERNAL_VERTICAL_PADDING = (FLATUI_BUTTONBAR_TARGET_HEIGHT - FLATUI_BUTTONBAR_ICON_SIZE) / 2;
// Additional constants for ICON_TEXT_BELOW style
constexpr int ICON_TEXT_BELOW_TOP_MARGIN = 2;     // Top margin above icon
constexpr int ICON_TEXT_BELOW_SPACING = 1;        // Space between icon and text
// constexpr int ICON_TEXT_BELOW_BOTTOM_MARGIN = 2; // Bottom margin below text (derived if total height is fixed)

FlatUIButtonBar::FlatUIButtonBar(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_DEFAULT),
      m_displayStyle(ButtonDisplayStyle::ICON_TEXT_BESIDE), // Default display style
      m_buttonStyle(ButtonStyle::DEFAULT),
      m_buttonBorderStyle(ButtonBorderStyle::SOLID),
      m_buttonBgColour(FLATUI_ACT_BAR_BACKGROUND_COLOUR),
      m_buttonHoverBgColour(FLATUI_BUTTONBAR_DEFAULT_HOVER_BG_COLOUR),
      m_buttonPressedBgColour(FLATUI_BUTTONBAR_DEFAULT_PRESSED_BG_COLOUR),
      m_buttonTextColour(FLATUI_BUTTONBAR_DEFAULT_TEXT_COLOUR),
      m_buttonBorderColour(FLATUI_BUTTONBAR_DEFAULT_BORDER_COLOUR),
      m_barBgColour(FLATUI_ACT_BAR_BACKGROUND_COLOUR),
      m_barBorderColour(FLATUI_ACT_BAR_BACKGROUND_COLOUR),
      m_buttonBorderWidth(FLATUI_BUTTONBAR_DEFAULT_BORDER_WIDTH),
      m_buttonCornerRadius(FLATUI_BUTTONBAR_DEFAULT_CORNER_RADIUS),
      m_buttonSpacing(FLATUI_BUTTONBAR_SPACING),
      m_buttonHorizontalPadding(FLATUI_BUTTONBAR_HORIZONTAL_PADDING),
      m_buttonVerticalPadding(FLATUI_BUTTONBAR_INTERNAL_VERTICAL_PADDING),
      m_barBorderWidth(0),
      m_hoverEffectsEnabled(true),
      m_hoveredButtonIndex(-1)
{
    SetFont(GetFlatUIDefaultFont());
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(FLATUI_BUTTONBAR_TARGET_HEIGHT * 2, FLATUI_BUTTONBAR_TARGET_HEIGHT));
    Bind(wxEVT_PAINT, &FlatUIButtonBar::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FlatUIButtonBar::OnMouseDown, this);
    Bind(wxEVT_MOTION, &FlatUIButtonBar::OnMouseMove, this);
    Bind(wxEVT_LEAVE_WINDOW, &FlatUIButtonBar::OnMouseLeave, this);
    Bind(wxEVT_SIZE, &FlatUIButtonBar::OnSize, this);
    // TODO: Add EVT_MOTION and EVT_LEAVE_WINDOW for hover effects if desired later
}

FlatUIButtonBar::~FlatUIButtonBar()
{
}

void FlatUIButtonBar::RecalculateLayout()
{
    Freeze();
    wxClientDC dc(this); 
    dc.SetFont(GetFont()); // Ensure correct font for measurements
    int currentX = FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN;

    for (auto& button : m_buttons) {
        int buttonWidth = 0;
        wxSize textSize = dc.GetTextExtent(button.label);
        int iconWidth = button.icon.IsOk() ? button.icon.GetWidth() : 0;
        int iconHeight = button.icon.IsOk() ? button.icon.GetHeight() : 0;

        switch (m_displayStyle) {
            case ButtonDisplayStyle::ICON_ONLY:
                if (button.icon.IsOk()) {
                    buttonWidth = iconWidth + 2 * m_buttonHorizontalPadding;
                } else { // Fallback if no icon, show text or be very small
                    buttonWidth = 2 * m_buttonHorizontalPadding; // Minimal width
                }
                break;
            case ButtonDisplayStyle::TEXT_ONLY:
                if (!button.label.empty()) {
                    buttonWidth = textSize.GetWidth() + 2 * m_buttonHorizontalPadding;
                } else { // Fallback if no text
                    buttonWidth = 2 * m_buttonHorizontalPadding; // Minimal width
                }
                break;
            case ButtonDisplayStyle::ICON_TEXT_BELOW:
                buttonWidth = m_buttonHorizontalPadding; // Left padding
                if (button.icon.IsOk() && !button.label.empty()) {
                    buttonWidth += wxMax(iconWidth, textSize.GetWidth());
                } else if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                } else if (!button.label.empty()) {
                    buttonWidth += textSize.GetWidth();
                } else {
                    // Neither icon nor text
                }
                buttonWidth += m_buttonHorizontalPadding; // Right padding
                break;
            case ButtonDisplayStyle::ICON_TEXT_BESIDE:
            default: // Default to ICON_TEXT_BESIDE
                buttonWidth += m_buttonHorizontalPadding; // Left padding
                if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                }
                if (!button.label.empty()) {
                    if (button.icon.IsOk()) {
                        buttonWidth += m_buttonHorizontalPadding; // Space between icon and text
                    }
                    buttonWidth += textSize.GetWidth();
                }
                buttonWidth += m_buttonHorizontalPadding; // Right padding
                break;
        }
        if (button.isDropDown) {
            buttonWidth += m_buttonHorizontalPadding + 6; // Extra space for dropdown arrow
        }
        if (buttonWidth == 0) buttonWidth = 2 * m_buttonHorizontalPadding; // Ensure a minimal clickable area

        button.rect = wxRect(currentX, 0, buttonWidth, FLATUI_BUTTONBAR_TARGET_HEIGHT);
        currentX += buttonWidth;
        if (&button != &m_buttons.back()) { 
            currentX += m_buttonSpacing;
        }
    }
    currentX += FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN; 

    wxSize currentMinSize = GetMinSize();
    if (currentMinSize.GetWidth() != currentX || currentMinSize.GetHeight() != FLATUI_BUTTONBAR_TARGET_HEIGHT) {
        SetMinSize(wxSize(currentX, FLATUI_BUTTONBAR_TARGET_HEIGHT));
        InvalidateBestSize(); 
        
        wxWindow* parentPanel = GetParent();
        if (parentPanel) {
            FlatUIPanel* panel = dynamic_cast<FlatUIPanel*>(parentPanel);
            if (panel) {
                panel->UpdatePanelSize();
            } else {
                parentPanel->Layout(); 
            }
        }
    }
    Thaw();
    Refresh(); 
}

void FlatUIButtonBar::AddButton(int id, const wxString& label, const wxBitmap& bitmap, wxMenu* menu)
{
    Freeze(); // Freeze before making multiple changes
    ButtonInfo button;
    button.id = id;
    button.label = label;
    if (bitmap.IsOk() && (bitmap.GetWidth() != 24 || bitmap.GetHeight() != 24)) {
        button.icon = wxBitmap(bitmap.ConvertToImage().Rescale(24, 24));
    } else {
        button.icon = bitmap;
    }
    button.menu = menu;
    button.isDropDown = (menu != nullptr);
    button.hovered = false; 
    // button.rect will be calculated in RecalculateLayout

    m_buttons.push_back(button);
    RecalculateLayout(); // Update layout and minsize

    Thaw(); // Thaw after all changes
    Refresh(); // Ensure repaint

    LOG_INF("Added button \"" + label.ToStdString() + "\" to ButtonBar. New BestSize: " +
        std::to_string(GetBestSize().GetWidth()) + "x" + std::to_string(GetBestSize().GetHeight()), "FlatUIButtonBar");
}

void FlatUIButtonBar::SetDisplayStyle(ButtonDisplayStyle style)
{
    if (m_displayStyle != style)
    {
        m_displayStyle = style;
        RecalculateLayout(); // Recalculate layout with the new style
        Refresh();           // Redraw the control
    }
}

wxSize FlatUIButtonBar::DoGetBestSize() const
{
    int totalWidth = FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN; 
    wxMemoryDC tempDC; 
    tempDC.SetFont(GetFlatUIDefaultFont());

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const auto& button = m_buttons[i];
        int buttonWidth = 0;
        wxSize textSize = tempDC.GetTextExtent(button.label);
        int iconWidth = button.icon.IsOk() ? button.icon.GetWidth() : 0;

        switch (m_displayStyle) {
            case ButtonDisplayStyle::ICON_ONLY:
                if (button.icon.IsOk()) {
                    buttonWidth = iconWidth + 2 * m_buttonHorizontalPadding;
                } else {
                    buttonWidth = 2 * m_buttonHorizontalPadding;
                }
                break;
            case ButtonDisplayStyle::TEXT_ONLY:
                if (!button.label.empty()) {
                    buttonWidth = textSize.GetWidth() + 2 * m_buttonHorizontalPadding;
                } else {
                    buttonWidth = 2 * m_buttonHorizontalPadding;
                }
                break;
            case ButtonDisplayStyle::ICON_TEXT_BELOW:
                buttonWidth = m_buttonHorizontalPadding; // Left padding
                if (button.icon.IsOk() && !button.label.empty()) {
                    buttonWidth += wxMax(iconWidth, textSize.GetWidth());
                } else if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                } else if (!button.label.empty()) {
                    buttonWidth += textSize.GetWidth();
                } else {
                    // Neither icon nor text
                }
                buttonWidth += m_buttonHorizontalPadding; // Right padding
                break;
            case ButtonDisplayStyle::ICON_TEXT_BESIDE:
            default:
                buttonWidth += m_buttonHorizontalPadding;
                if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                }
                if (!button.label.empty()) {
                    if (button.icon.IsOk()) {
                        buttonWidth += m_buttonHorizontalPadding;
                    }
                    buttonWidth += textSize.GetWidth();
                }
                buttonWidth += m_buttonHorizontalPadding;
                break;
        }
        if (buttonWidth == 0) buttonWidth = 2 * m_buttonHorizontalPadding; // Minimal width

        totalWidth += buttonWidth;
        if (i < m_buttons.size() - 1) { 
            totalWidth += m_buttonSpacing;
        }
    }
    totalWidth += FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN; 

    if (m_buttons.empty()){ 
        totalWidth = FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN * 2;
        if (totalWidth == 0) totalWidth = 10; 
    }

    return wxSize(totalWidth, FLATUI_BUTTONBAR_TARGET_HEIGHT);
}

void FlatUIButtonBar::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize controlSize = GetSize(); 

    // Background of the bar itself
    dc.SetBackground(FLATUI_ACT_BAR_BACKGROUND_COLOUR);
    dc.Clear();

    if (m_buttons.empty() && IsShown()) { // Only draw placeholder if shown and empty
        dc.SetTextForeground(wxColour(120, 120, 120)); // Lighter grey for placeholder
        wxString text = "Button Bar"; // More descriptive placeholder
        wxSize textSize = dc.GetTextExtent(text);
        // Center placeholder text
        int x = (controlSize.GetWidth() - textSize.GetWidth()) / 2;
        int y = (controlSize.GetHeight() - textSize.GetHeight()) / 2;
        dc.DrawText(text, x, y);
        return; // Nothing else to paint if empty
    }

    dc.SetFont(GetFont()); // Ensure correct font is used for text measurements and drawing

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        DrawButton(dc, m_buttons[i], i);
    }
}

void FlatUIButtonBar::DrawButton(wxDC& dc, const ButtonInfo& button, int index)
{
    wxRect buttonRect = button.rect; 
    bool isHovered = (m_hoverEffectsEnabled && index == m_hoveredButtonIndex);
    bool isPressed = button.pressed;
    
    // Draw button background
    if (m_buttonStyle != ButtonStyle::GHOST || isHovered || isPressed) {
        DrawButtonBackground(dc, buttonRect, isHovered, isPressed);
    }
    
    // Draw button border
    if (m_buttonStyle == ButtonStyle::OUTLINED || 
        m_buttonStyle == ButtonStyle::RAISED ||
        (m_buttonStyle == ButtonStyle::DEFAULT && (isHovered || isPressed))) {
        DrawButtonBorder(dc, buttonRect, isHovered, isPressed);
    }
    
    // Draw button content
    wxSize textSize = dc.GetTextExtent(button.label);
    int iconHeight = button.icon.IsOk() ? button.icon.GetHeight() : 0;
    int iconWidth = button.icon.IsOk() ? button.icon.GetWidth() : 0;

    dc.SetTextForeground(m_buttonTextColour);
    
    switch (m_displayStyle) {
        case ButtonDisplayStyle::ICON_ONLY:
            if (button.icon.IsOk()) {
                int iconX = buttonRect.GetLeft() + (buttonRect.GetWidth() - iconWidth) / 2;
                int iconY = buttonRect.GetTop() + (buttonRect.GetHeight() - iconHeight) / 2;
                dc.DrawBitmap(button.icon, iconX, iconY, true);
            }
            break;
        case ButtonDisplayStyle::TEXT_ONLY:
            if (!button.label.empty()) {
                int textX = buttonRect.GetLeft() + (buttonRect.GetWidth() - textSize.GetWidth()) / 2;
                int textY = buttonRect.GetTop() + (buttonRect.GetHeight() - textSize.GetHeight()) / 2;
                dc.DrawText(button.label, textX, textY);
            }
            break;
        case ButtonDisplayStyle::ICON_TEXT_BELOW:
        {
            int currentY = buttonRect.GetTop() + ICON_TEXT_BELOW_TOP_MARGIN;
            if (button.icon.IsOk()) {
                int iconX = buttonRect.GetLeft() + (buttonRect.GetWidth() - iconWidth) / 2;
                dc.DrawBitmap(button.icon, iconX, currentY, true);
                currentY += iconHeight + ICON_TEXT_BELOW_SPACING;
            }
            if (!button.label.empty()) {
                int textX = buttonRect.GetLeft() + (buttonRect.GetWidth() - textSize.GetWidth()) / 2;
                // Ensure text is not drawn outside buttonRect if it's too tall
                if (currentY + textSize.GetHeight() <= buttonRect.GetBottom()) {
                     dc.DrawText(button.label, textX, currentY);
                }
            }
        }
            break;
        case ButtonDisplayStyle::ICON_TEXT_BESIDE:
        default:
        {
            int currentX = buttonRect.GetLeft() + m_buttonHorizontalPadding;
            if (button.icon.IsOk()) {
                int iconY = buttonRect.GetTop() + (buttonRect.GetHeight() - iconHeight) / 2; 
                dc.DrawBitmap(button.icon, currentX, iconY, true);
                currentX += iconWidth;
            }
            if (!button.label.empty()) {
                if (button.icon.IsOk()) { 
                    currentX += m_buttonHorizontalPadding;
                }
                int textY = buttonRect.GetTop() + (buttonRect.GetHeight() - textSize.GetHeight()) / 2;
                dc.DrawText(button.label, currentX, textY);
            }
        }
            break;
    }
    
    // Draw Dropdown arrow if it's a dropdown button (common for all styles)
    if (button.isDropDown) {
        // Position arrow consistently at the right edge, vertically centered.
        // The button.rect.GetRight() is the exclusive end, so subtract arrow width and padding.
        int arrowWidth = 5; // Approximate width of the arrow glyph
        int arrowHeight = 3; // Approximate height of the arrow glyph
        int arrowX = button.rect.GetRight() - m_buttonHorizontalPadding - arrowWidth;
        int arrowY = buttonRect.GetTop() + (buttonRect.GetHeight() - arrowHeight) / 2;
        wxPoint points[3];
        points[0] = wxPoint(arrowX, arrowY);
        points[1] = wxPoint(arrowX + arrowWidth, arrowY);
        points[2] = wxPoint(arrowX + (arrowWidth / 2), arrowY + arrowHeight);
        dc.SetBrush(wxBrush(m_buttonTextColour)); 
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawPolygon(3, points);
    }
}

void FlatUIButtonBar::DrawButtonBackground(wxDC& dc, const wxRect& rect, bool isHovered, bool isPressed)
{
    wxColour bgColour = m_buttonBgColour;
    if (isPressed && m_hoverEffectsEnabled) {
        bgColour = m_buttonPressedBgColour;
    } else if (isHovered && m_hoverEffectsEnabled) {
        bgColour = m_buttonHoverBgColour;
    }
    
    dc.SetBrush(wxBrush(bgColour));
    dc.SetPen(*wxTRANSPARENT_PEN);
    
    if (m_buttonStyle == ButtonStyle::PILL || 
        (m_buttonBorderStyle == ButtonBorderStyle::ROUNDED && m_buttonCornerRadius > 0)) {
        // Draw rounded rectangle
        dc.DrawRoundedRectangle(rect, m_buttonCornerRadius);
    } else {
        dc.DrawRectangle(rect);
    }
    
    // Add shadow effect for RAISED style
    if (m_buttonStyle == ButtonStyle::RAISED && !isPressed) {
        wxColour shadowColour = bgColour.ChangeLightness(70);
        dc.SetPen(wxPen(shadowColour, 1));
        dc.DrawLine(rect.GetLeft() + 1, rect.GetBottom(), 
                   rect.GetRight(), rect.GetBottom());
        dc.DrawLine(rect.GetRight(), rect.GetTop() + 1, 
                   rect.GetRight(), rect.GetBottom());
    }
}

void FlatUIButtonBar::DrawButtonBorder(wxDC& dc, const wxRect& rect, bool isHovered, bool isPressed)
{
    wxColour borderColour = m_buttonBorderColour;
    if (isHovered && m_hoverEffectsEnabled) {
        borderColour = borderColour.ChangeLightness(80);
    }
    wxRect innerRect = rect;
    switch (m_buttonBorderStyle) {
        case ButtonBorderStyle::SOLID:
            dc.SetPen(wxPen(borderColour, m_buttonBorderWidth));
            break;
        case ButtonBorderStyle::DASHED:
            dc.SetPen(wxPen(borderColour, m_buttonBorderWidth, wxPENSTYLE_SHORT_DASH));
            break;
        case ButtonBorderStyle::DOTTED:
            dc.SetPen(wxPen(borderColour, m_buttonBorderWidth, wxPENSTYLE_DOT));
            break;
        case ButtonBorderStyle::DOUBLE:
        {
            // Draw double border
            dc.SetPen(wxPen(borderColour, 1));
            dc.DrawRectangle(rect);
            innerRect.Deflate(2);
            dc.DrawRectangle(innerRect);
            return;
        }
        case ButtonBorderStyle::ROUNDED:
            dc.SetPen(wxPen(borderColour, m_buttonBorderWidth));
            break;
    }
    
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    
    if (m_buttonStyle == ButtonStyle::PILL || 
        (m_buttonBorderStyle == ButtonBorderStyle::ROUNDED && m_buttonCornerRadius > 0)) {
        dc.DrawRoundedRectangle(rect, m_buttonCornerRadius);
    } else {
        dc.DrawRectangle(rect);
    }
}

void FlatUIButtonBar::OnMouseMove(wxMouseEvent& evt)
{
    if (!m_hoverEffectsEnabled) {
        evt.Skip();
        return;
    }
    
    wxPoint pos = evt.GetPosition();
    int oldHoveredIndex = m_hoveredButtonIndex;
    m_hoveredButtonIndex = -1;
    
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].rect.Contains(pos)) {
            m_hoveredButtonIndex = i;
            break;
        }
    }
    
    if (oldHoveredIndex != m_hoveredButtonIndex) {
        Refresh();
    }
    
    evt.Skip();
}

void FlatUIButtonBar::OnMouseLeave(wxMouseEvent& evt)
{
    if (m_hoveredButtonIndex != -1) {
        m_hoveredButtonIndex = -1;
        Refresh();
    }
    evt.Skip();
}

void FlatUIButtonBar::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();

    for (auto& button : m_buttons) {
        if (button.rect.Contains(pos)) {
            if (button.menu) {
                wxPoint menuPos = ClientToScreen(button.rect.GetBottomLeft());
                PopupMenu(button.menu, menuPos.x, menuPos.y);
            }
            else {
                wxCommandEvent event(wxEVT_BUTTON, button.id);
                event.SetEventObject(this);
                GetParent()->ProcessWindowEvent(event);
            }
            break;
        }
    }

    evt.Skip();
}

void FlatUIButtonBar::OnSize(wxSizeEvent& evt)
{
    Refresh();
    evt.Skip();
}

// Style configuration method implementations
void FlatUIButtonBar::SetButtonStyle(ButtonStyle style)
{
    if (m_buttonStyle != style) {
        m_buttonStyle = style;
        Refresh();
    }
}

void FlatUIButtonBar::SetButtonBorderStyle(ButtonBorderStyle style)
{
    if (m_buttonBorderStyle != style) {
        m_buttonBorderStyle = style;
        Refresh();
    }
}

void FlatUIButtonBar::SetButtonBackgroundColour(const wxColour& colour)
{
    m_buttonBgColour = colour;
    Refresh();
}

void FlatUIButtonBar::SetButtonHoverBackgroundColour(const wxColour& colour)
{
    m_buttonHoverBgColour = colour;
    Refresh();
}

void FlatUIButtonBar::SetButtonPressedBackgroundColour(const wxColour& colour)
{
    m_buttonPressedBgColour = colour;
    Refresh();
}

void FlatUIButtonBar::SetButtonTextColour(const wxColour& colour)
{
    m_buttonTextColour = colour;
    Refresh();
}

void FlatUIButtonBar::SetButtonBorderColour(const wxColour& colour)
{
    m_buttonBorderColour = colour;
    Refresh();
}

void FlatUIButtonBar::SetButtonBorderWidth(int width)
{
    m_buttonBorderWidth = width;
    Refresh();
}

void FlatUIButtonBar::SetButtonCornerRadius(int radius)
{
    m_buttonCornerRadius = radius;
    Refresh();
}

void FlatUIButtonBar::SetButtonSpacing(int spacing)
{
    if (m_buttonSpacing != spacing) {
        m_buttonSpacing = spacing;
        RecalculateLayout();
    }
}

void FlatUIButtonBar::SetButtonPadding(int horizontal, int vertical)
{
    if (m_buttonHorizontalPadding != horizontal || m_buttonVerticalPadding != vertical) {
        m_buttonHorizontalPadding = horizontal;
        m_buttonVerticalPadding = vertical;
        RecalculateLayout();
    }
}

void FlatUIButtonBar::GetButtonPadding(int& horizontal, int& vertical) const
{
    horizontal = m_buttonHorizontalPadding;
    vertical = m_buttonVerticalPadding;
}

void FlatUIButtonBar::SetBarBackgroundColour(const wxColour& colour)
{
    m_barBgColour = colour;
    Refresh();
}

void FlatUIButtonBar::SetBarBorderColour(const wxColour& colour)
{
    m_barBorderColour = colour;
    Refresh();
}

void FlatUIButtonBar::SetBarBorderWidth(int width)
{
    m_barBorderWidth = width;
    Refresh();
}

void FlatUIButtonBar::SetHoverEffectsEnabled(bool enabled)
{
    if (m_hoverEffectsEnabled != enabled) {
        m_hoverEffectsEnabled = enabled;
        m_hoveredButtonIndex = -1;
        Refresh();
    }
}