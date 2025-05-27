#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIConstants.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIEventManager.h"
#include <wx/dcbuffer.h>
#include <wx/graphics.h>
#include <wx/display.h>
#include <algorithm> // For std::min

// Constants
constexpr int ICON_TEXT_BELOW_TOP_MARGIN = 2;
constexpr int ICON_TEXT_BELOW_SPACING = 1;

FlatUIButtonBar::FlatUIButtonBar(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_displayStyle(ButtonDisplayStyle::ICON_TEXT_BESIDE),
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
    m_hoverEffectsEnabled(true)
{
    SetFont(GetFlatUIDefaultFont());
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(FLATUI_BUTTONBAR_TARGET_HEIGHT * 2, FLATUI_BUTTONBAR_TARGET_HEIGHT));
    Bind(wxEVT_PAINT, &FlatUIButtonBar::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FlatUIButtonBar::OnMouseDown, this);
    Bind(wxEVT_MOTION, &FlatUIButtonBar::OnMouseMove, this);
    Bind(wxEVT_LEAVE_WINDOW, &FlatUIButtonBar::OnMouseLeave, this);
    Bind(wxEVT_SIZE, &FlatUIButtonBar::OnSize, this);
}

FlatUIButtonBar::~FlatUIButtonBar() = default;

void FlatUIButtonBar::AddButton(int id, const wxString& label, const wxBitmap& bitmap, wxMenu* menu)
{
    Freeze();
    ButtonInfo button;
    button.id = id;
    button.label = label;
    int imgw = FLATUI_BUTTONBAR_ICON_SIZE;
    if (bitmap.IsOk() && (bitmap.GetWidth() != imgw || bitmap.GetHeight() != imgw)) {
        button.icon = wxBitmap(bitmap.ConvertToImage().Rescale(imgw, imgw, wxIMAGE_QUALITY_HIGH));
    }
    else {
        button.icon = bitmap;
    }
    button.menu = menu;
    button.isDropDown = (menu != nullptr);

    wxClientDC dc(this);
    dc.SetFont(GetFont());
    button.textSize = dc.GetTextExtent(label);

    m_buttons.push_back(button);
    RecalculateLayout();

    Thaw();
    Refresh();

    LOG_INF(wxString::Format("Added button \"%s\" to ButtonBar. BestSize: %dx%d",
        label, GetBestSize().GetWidth(), GetBestSize().GetHeight()).ToStdString(),
        "FlatUIButtonBar");
}

int FlatUIButtonBar::CalculateButtonWidth(const ButtonInfo& button, wxDC& dc) const
{
    int buttonWidth = 0;
    int iconWidth = button.icon.IsOk() ? button.icon.GetWidth() : 0;

    switch (m_displayStyle) {
    case ButtonDisplayStyle::ICON_ONLY:
        buttonWidth = button.icon.IsOk() ? iconWidth + 2 * m_buttonHorizontalPadding
            : 2 * m_buttonHorizontalPadding;
        break;
    case ButtonDisplayStyle::TEXT_ONLY:
        buttonWidth = !button.label.empty() ? button.textSize.GetWidth() + 2 * m_buttonHorizontalPadding
            : 2 * m_buttonHorizontalPadding;
        break;
    case ButtonDisplayStyle::ICON_TEXT_BELOW:
        buttonWidth = m_buttonHorizontalPadding;
        if (button.icon.IsOk() && !button.label.empty()) {
            buttonWidth += wxMax(iconWidth, button.textSize.GetWidth());
        }
        else if (button.icon.IsOk()) {
            buttonWidth += iconWidth;
        }
        else if (!button.label.empty()) {
            buttonWidth += button.textSize.GetWidth();
        }
        buttonWidth += m_buttonHorizontalPadding;
        break;
    case ButtonDisplayStyle::ICON_TEXT_BESIDE:
        buttonWidth = m_buttonHorizontalPadding;
        if (button.icon.IsOk()) {
            buttonWidth += iconWidth;
        }
        if (!button.label.empty()) {
            if (button.icon.IsOk()) {
                buttonWidth += m_buttonHorizontalPadding;
            }
            buttonWidth += button.textSize.GetWidth();
        }
        buttonWidth += m_buttonHorizontalPadding;
        break;
    }

    if (button.isDropDown) {
        buttonWidth += m_buttonHorizontalPadding + FLATUI_BUTTONBAR_DROPDOWN_ARROW_WIDTH;
        // Add separator width and padding
        buttonWidth += FLATUI_BUTTONBAR_SEPARATOR_WIDTH + 2 * FLATUI_BUTTONBAR_SEPARATOR_PADDING;
    }
    return wxMax(buttonWidth, 2 * m_buttonHorizontalPadding);
}

void FlatUIButtonBar::RecalculateLayout()
{
    Freeze();
    wxClientDC dc(this);
    dc.SetFont(GetFont());
    int currentX = FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN;

    for (auto& button : m_buttons) {
        if (button.textSize != dc.GetTextExtent(button.label)) {
            button.textSize = dc.GetTextExtent(button.label);
        }
        int buttonWidth = CalculateButtonWidth(button, dc);
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
        if (auto* parentPanel = dynamic_cast<FlatUIPanel*>(GetParent())) {
            parentPanel->UpdatePanelSize();
        }
        else {
            GetParent()->Layout();
        }
    }
    Thaw();
    Refresh();
}

void FlatUIButtonBar::SetDisplayStyle(ButtonDisplayStyle style)
{
    if (m_displayStyle != style) {
        m_displayStyle = style;
        RecalculateLayout();
        Refresh();
    }
}

wxSize FlatUIButtonBar::DoGetBestSize() const
{
    wxMemoryDC dc;
    dc.SetFont(GetFlatUIDefaultFont());
    int totalWidth = FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN;

    for (const auto& button : m_buttons) {
        totalWidth += CalculateButtonWidth(button, dc);
        if (&button != &m_buttons.back()) {
            totalWidth += m_buttonSpacing;
        }
    }
    totalWidth += FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN;

    if (m_buttons.empty()) {
        totalWidth = FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN * 2;
        if (totalWidth == 0) totalWidth = 10;
    }

    return wxSize(totalWidth, FLATUI_BUTTONBAR_TARGET_HEIGHT);
}

void FlatUIButtonBar::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    dc.SetBackground(m_barBgColour);
    dc.Clear();

    if (m_buttons.empty() && IsShown()) {
        dc.SetTextForeground(wxColour(120, 120, 120));
        wxString text = "Button Bar";
        wxSize textSize = dc.GetTextExtent(text);
        dc.DrawText(text, (GetSize().GetWidth() - textSize.GetWidth()) / 2,
            (GetSize().GetHeight() - textSize.GetHeight()) / 2);
        return;
    }

    dc.SetFont(GetFont());
    for (size_t i = 0; i < m_buttons.size(); ++i) {
        DrawButton(dc, m_buttons[i], i);
    }
}

void FlatUIButtonBar::DrawButton(wxDC& dc, const ButtonInfo& button, int index)
{
    bool isHovered = m_hoverEffectsEnabled && index == m_hoveredButtonIndex;
    bool isPressed = button.pressed;

    if (m_buttonStyle != ButtonStyle::GHOST || isHovered || isPressed) {
        DrawButtonBackground(dc, button.rect, isHovered, isPressed);
    }

    if (m_buttonStyle == ButtonStyle::OUTLINED ||
        m_buttonStyle == ButtonStyle::RAISED ||
        (m_buttonStyle == ButtonStyle::DEFAULT && (isHovered || isPressed))) {
        DrawButtonBorder(dc, button.rect, isHovered, isPressed);
    }

    dc.SetTextForeground(m_buttonTextColour);
    DrawButtonIcon(dc, button, button.rect);
    DrawButtonText(dc, button, button.rect);
    if (button.isDropDown) {
        DrawButtonSeparator(dc, button, button.rect); // Draw separator before arrow
        DrawButtonDropdownArrow(dc, button, button.rect);
    }
}

void FlatUIButtonBar::DrawButtonIcon(wxDC& dc, const ButtonInfo& button, const wxRect& rect)
{
    if (!button.icon.IsOk()) return;

    int iconWidth = button.icon.GetWidth();
    int iconHeight = button.icon.GetHeight();

    switch (m_displayStyle) {
    case ButtonDisplayStyle::ICON_ONLY:
    {
        int iconX = rect.GetLeft() + (rect.GetWidth() - iconWidth) / 2;
        int iconY = rect.GetTop() + (rect.GetHeight() - iconHeight) / 2;
        dc.DrawBitmap(button.icon, iconX, iconY, true);
        break;
    }
    case ButtonDisplayStyle::ICON_TEXT_BELOW:
    {
        int iconX = rect.GetLeft() + (rect.GetWidth() - iconWidth) / 2;
        int iconY = rect.GetTop() + ICON_TEXT_BELOW_TOP_MARGIN;
        dc.DrawBitmap(button.icon, iconX, iconY, true);
        break;
    }
    case ButtonDisplayStyle::ICON_TEXT_BESIDE:
    {
        int iconX = rect.GetLeft() + m_buttonHorizontalPadding;
        int iconY = rect.GetTop() + (rect.GetHeight() - iconHeight) / 2;
        dc.DrawBitmap(button.icon, iconX, iconY, true);
        break;
    }
    default:
        break;
    }
}

void FlatUIButtonBar::DrawButtonText(wxDC& dc, const ButtonInfo& button, const wxRect& rect)
{
    if (button.label.empty()) return;

    switch (m_displayStyle) {
    case ButtonDisplayStyle::TEXT_ONLY:
    {
        int textX = rect.GetLeft() + (rect.GetWidth() - button.textSize.GetWidth()) / 2;
        int textY = rect.GetTop() + (rect.GetHeight() - button.textSize.GetHeight()) / 2;
        dc.DrawText(button.label, textX, textY);
        break;
    }
    case ButtonDisplayStyle::ICON_TEXT_BELOW:
    {
        int textX = rect.GetLeft() + (rect.GetWidth() - button.textSize.GetWidth()) / 2;
        int textY = rect.GetTop() + ICON_TEXT_BELOW_TOP_MARGIN + (button.icon.IsOk() ? button.icon.GetHeight() + ICON_TEXT_BELOW_SPACING : 0);
        if (textY + button.textSize.GetHeight() <= rect.GetBottom()) {
            dc.DrawText(button.label, textX, textY);
        }
        break;
    }
    case ButtonDisplayStyle::ICON_TEXT_BESIDE:
    {
        int textX = rect.GetLeft() + m_buttonHorizontalPadding + (button.icon.IsOk() ? button.icon.GetWidth() + m_buttonHorizontalPadding : 0);
        int textY = rect.GetTop() + (rect.GetHeight() - button.textSize.GetHeight()) / 2;
        dc.DrawText(button.label, textX, textY);
        break;
    }
    default:
        break;
    }
}

void FlatUIButtonBar::DrawButtonDropdownArrow(wxDC& dc, const ButtonInfo& button, const wxRect& rect)
{
    int arrowX = rect.GetRight() - m_buttonHorizontalPadding - FLATUI_BUTTONBAR_DROPDOWN_ARROW_WIDTH;
    int arrowY = rect.GetTop() + (rect.GetHeight() - FLATUI_BUTTONBAR_DROPDOWN_ARROW_HEIGHT) / 2;
    wxPoint points[3] = {
        {arrowX, arrowY},
        {arrowX + FLATUI_BUTTONBAR_DROPDOWN_ARROW_WIDTH, arrowY},
        {arrowX + FLATUI_BUTTONBAR_DROPDOWN_ARROW_WIDTH / 2, arrowY + FLATUI_BUTTONBAR_DROPDOWN_ARROW_HEIGHT}
    };
    dc.SetBrush(wxBrush(m_buttonTextColour));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawPolygon(3, points);
}

void FlatUIButtonBar::DrawButtonSeparator(wxDC& dc, const ButtonInfo& button, const wxRect& rect)
{
    // Calculate separator position: before the dropdown arrow
    int separatorX = rect.GetRight() - m_buttonHorizontalPadding - FLATUI_BUTTONBAR_DROPDOWN_ARROW_WIDTH
        - FLATUI_BUTTONBAR_SEPARATOR_PADDING - FLATUI_BUTTONBAR_SEPARATOR_WIDTH;
    int topY = rect.GetTop() + FLATUI_BUTTONBAR_SEPARATOR_MARGIN;
    int bottomY = rect.GetBottom() - FLATUI_BUTTONBAR_SEPARATOR_MARGIN;

    dc.SetPen(wxPen(FLATUI_BUTTONBAR_DEFAULT_BORDER_COLOUR, FLATUI_BUTTONBAR_SEPARATOR_WIDTH));
    dc.DrawLine(separatorX, topY, separatorX, bottomY);

    LOG_INF(wxString::Format("Drawing separator at x=%d, y=%d to %d for button \"%s\"",
        separatorX, topY, bottomY, button.label).ToStdString(),
        "FlatUIButtonBar");
}

void FlatUIButtonBar::DrawButtonBackground(wxDC& dc, const wxRect& rect, bool isHovered, bool isPressed)
{
    wxColour bgColour = isPressed && m_hoverEffectsEnabled ? m_buttonPressedBgColour :
        isHovered && m_hoverEffectsEnabled ? m_buttonHoverBgColour :
        m_buttonBgColour;

    dc.SetBrush(wxBrush(bgColour));
    dc.SetPen(*wxTRANSPARENT_PEN);

    if (m_buttonStyle == ButtonStyle::PILL ||
        (m_buttonBorderStyle == ButtonBorderStyle::ROUNDED && m_buttonCornerRadius > 0)) {
        dc.DrawRoundedRectangle(rect, m_buttonCornerRadius);
    }
    else {
        dc.DrawRectangle(rect);
    }

    if (m_buttonStyle == ButtonStyle::RAISED && !isPressed) {
        wxColour shadowColour = bgColour.ChangeLightness(70);
        dc.SetPen(wxPen(shadowColour, 1));
        dc.DrawLine(rect.GetLeft() + 1, rect.GetBottom(), rect.GetRight(), rect.GetBottom());
        dc.DrawLine(rect.GetRight(), rect.GetTop() + 1, rect.GetRight(), rect.GetBottom());
    }
}

void FlatUIButtonBar::DrawButtonBorder(wxDC& dc, const wxRect& rect, bool isHovered, bool isPressed)
{
    wxColour borderColour = isHovered && m_hoverEffectsEnabled ? m_buttonBorderColour.ChangeLightness(80)
        : m_buttonBorderColour;
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
        dc.SetPen(wxPen(borderColour, 1));
        dc.DrawRectangle(rect);
        innerRect.Deflate(2);
        dc.DrawRectangle(innerRect);
        return;
    case ButtonBorderStyle::ROUNDED:
        dc.SetPen(wxPen(borderColour, m_buttonBorderWidth));
        break;
    }

    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    if (m_buttonStyle == ButtonStyle::PILL ||
        (m_buttonBorderStyle == ButtonBorderStyle::ROUNDED && m_buttonCornerRadius > 0)) {
        dc.DrawRoundedRectangle(rect, m_buttonCornerRadius);
    }
    else {
        dc.DrawRectangle(rect);
    }
}

void FlatUIButtonBar::OnMouseMove(wxMouseEvent& evt)
{
    if (!m_hoverEffectsEnabled) return;

    int oldHoveredIndex = m_hoveredButtonIndex;
    m_hoveredButtonIndex = -1;
    wxPoint pos = evt.GetPosition();

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        if (m_buttons[i].rect.Contains(pos)) {
            m_hoveredButtonIndex = i;
            break;
        }
    }

    if (oldHoveredIndex != m_hoveredButtonIndex) {
        Refresh();
    }
}

void FlatUIButtonBar::OnMouseLeave(wxMouseEvent& evt)
{
    if (m_hoveredButtonIndex != -1) {
        m_hoveredButtonIndex = -1;
        Refresh();
    }
}

void FlatUIButtonBar::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();

    for (const auto& button : m_buttons) {
        if (button.rect.Contains(pos)) {
            if (button.menu) {
                // Align menu with button's left-bottom corner
                wxPoint menuPos = button.rect.GetBottomLeft();
                menuPos.y += FLATUI_BUTTONBAR_MENU_VERTICAL_OFFSET;
                // Log client and screen coordinates
                wxPoint screenMenuPos = ClientToScreen(menuPos);
                LOG_INF(wxString::Format("Button rect: x=%d, y=%d, w=%d, h=%d",
                    button.rect.GetX(), button.rect.GetY(),
                    button.rect.GetWidth(), button.rect.GetHeight()).ToStdString(),
                    "FlatUIButtonBar");
                LOG_INF(wxString::Format("Client MenuPos: %d, %d; Screen MenuPos: %d, %d for button \"%s\"",
                    menuPos.x, menuPos.y, screenMenuPos.x, screenMenuPos.y, button.label).ToStdString(),
                    "FlatUIButtonBar");
                PopupMenu(button.menu, menuPos);
            }
            else {
                wxCommandEvent event(wxEVT_BUTTON, button.id);
                event.SetEventObject(this);
                GetParent()->ProcessWindowEvent(event);
            }
            break;
        }
    }
}

void FlatUIButtonBar::OnSize(wxSizeEvent& evt)
{
    RecalculateLayout();
}

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