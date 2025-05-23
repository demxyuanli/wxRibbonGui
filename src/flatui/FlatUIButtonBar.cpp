#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIConstants.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIEventManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>

// Calculate this one if needed, or ensure it's derived correctly where used
constexpr int FLATUI_BUTTONBAR_INTERNAL_VERTICAL_PADDING = (FLATUI_BUTTONBAR_TARGET_HEIGHT - FLATUI_BUTTONBAR_ICON_SIZE) / 2;
// Additional constants for ICON_TEXT_BELOW style
constexpr int ICON_TEXT_BELOW_TOP_MARGIN = 2;     // Top margin above icon
constexpr int ICON_TEXT_BELOW_SPACING = 1;        // Space between icon and text
// constexpr int ICON_TEXT_BELOW_BOTTOM_MARGIN = 2; // Bottom margin below text (derived if total height is fixed)

FlatUIButtonBar::FlatUIButtonBar(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY),
      m_displayStyle(ButtonDisplayStyle::ICON_TEXT_BESIDE) // Default display style
{
    SetFont(GetFlatUIDefaultFont());
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(FLATUI_BUTTONBAR_BAR_HORIZONTAL_MARGIN * 2, FLATUI_BUTTONBAR_TARGET_HEIGHT)); 
    Bind(wxEVT_PAINT, &FlatUIButtonBar::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FlatUIButtonBar::OnMouseDown, this);
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

        switch (m_displayStyle) {
            case ButtonDisplayStyle::ICON_ONLY:
                if (button.icon.IsOk()) {
                    buttonWidth = iconWidth + 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                } else { // Fallback if no icon, show text or be very small
                    buttonWidth = 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Minimal width
                }
                break;
            case ButtonDisplayStyle::TEXT_ONLY:
                if (!button.label.empty()) {
                    buttonWidth = textSize.GetWidth() + 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                } else { // Fallback if no text
                    buttonWidth = 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Minimal width
                }
                break;
            case ButtonDisplayStyle::ICON_TEXT_BELOW:
                buttonWidth = FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Left padding
                if (button.icon.IsOk() && !button.label.empty()) {
                    buttonWidth += wxMax(iconWidth, textSize.GetWidth());
                } else if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                } else if (!button.label.empty()) {
                    buttonWidth += textSize.GetWidth();
                } else {
                    // Neither icon nor text
                }
                buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Right padding
                break;
            case ButtonDisplayStyle::ICON_TEXT_BESIDE:
            default: // Default to ICON_TEXT_BESIDE
                buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Left padding
                if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                }
                if (!button.label.empty()) {
                    if (button.icon.IsOk()) {
                        buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Space between icon and text
                    }
                    buttonWidth += textSize.GetWidth();
                }
                buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Right padding
                break;
        }
        if (buttonWidth == 0) buttonWidth = 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Ensure a minimal clickable area

        button.rect = wxRect(currentX, 0, buttonWidth, FLATUI_BUTTONBAR_TARGET_HEIGHT);
        currentX += buttonWidth;
        if (&button != &m_buttons.back()) { 
            currentX += FLATUI_BUTTONBAR_SPACING;
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
    button.icon = bitmap; // Should be 16x16
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
    tempDC.SetFont(GetFont()); 

    for (size_t i = 0; i < m_buttons.size(); ++i) {
        const auto& button = m_buttons[i];
        int buttonWidth = 0;
        wxSize textSize = tempDC.GetTextExtent(button.label);
        int iconWidth = button.icon.IsOk() ? button.icon.GetWidth() : 0;

        switch (m_displayStyle) {
            case ButtonDisplayStyle::ICON_ONLY:
                if (button.icon.IsOk()) {
                    buttonWidth = iconWidth + 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                } else {
                    buttonWidth = 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                }
                break;
            case ButtonDisplayStyle::TEXT_ONLY:
                if (!button.label.empty()) {
                    buttonWidth = textSize.GetWidth() + 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                } else {
                    buttonWidth = 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                }
                break;
            case ButtonDisplayStyle::ICON_TEXT_BELOW:
                buttonWidth = FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Left padding
                if (button.icon.IsOk() && !button.label.empty()) {
                    buttonWidth += wxMax(iconWidth, textSize.GetWidth());
                } else if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                } else if (!button.label.empty()) {
                    buttonWidth += textSize.GetWidth();
                } else {
                    // Neither icon nor text
                }
                buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Right padding
                break;
            case ButtonDisplayStyle::ICON_TEXT_BESIDE:
            default:
                buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                if (button.icon.IsOk()) {
                    buttonWidth += iconWidth;
                }
                if (!button.label.empty()) {
                    if (button.icon.IsOk()) {
                        buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                    }
                    buttonWidth += textSize.GetWidth();
                }
                buttonWidth += FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                break;
        }
        if (buttonWidth == 0) buttonWidth = 2 * FLATUI_BUTTONBAR_HORIZONTAL_PADDING; // Minimal width

        totalWidth += buttonWidth;
        if (i < m_buttons.size() - 1) { 
            totalWidth += FLATUI_BUTTONBAR_SPACING;
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
    dc.SetBackground(FLATUI_PRIMARY_CONTENT_BG_COLOUR); // Use consistent BG color
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

    for (const auto& button : m_buttons) {
        wxRect buttonRect = button.rect; 
        wxSize textSize = dc.GetTextExtent(button.label);
        int iconHeight = button.icon.IsOk() ? button.icon.GetHeight() : 0;
        int iconWidth = button.icon.IsOk() ? button.icon.GetWidth() : 0;

        // Optional: Draw button background for hover/press (not implemented yet)
        // if (button.hovered) { ... }

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
                    dc.SetTextForeground(*wxBLACK);
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
                         dc.SetTextForeground(*wxBLACK);
                         dc.DrawText(button.label, textX, currentY);
                    }
                }
            }
                break;
            case ButtonDisplayStyle::ICON_TEXT_BESIDE:
            default:
            {
                int currentX = buttonRect.GetLeft() + FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                if (button.icon.IsOk()) {
                    int iconY = buttonRect.GetTop() + (buttonRect.GetHeight() - iconHeight) / 2; 
                    dc.DrawBitmap(button.icon, currentX, iconY, true);
                    currentX += iconWidth;
                }
                if (!button.label.empty()) {
                    if (button.icon.IsOk()) { 
                        currentX += FLATUI_BUTTONBAR_HORIZONTAL_PADDING;
                    }
                    dc.SetTextForeground(*wxBLACK); 
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
            int arrowX = button.rect.GetRight() - FLATUI_BUTTONBAR_HORIZONTAL_PADDING - arrowWidth;
            int arrowY = buttonRect.GetTop() + (buttonRect.GetHeight() - arrowHeight) / 2;
            wxPoint points[3];
            points[0] = wxPoint(arrowX, arrowY);
            points[1] = wxPoint(arrowX + arrowWidth, arrowY);
            points[2] = wxPoint(arrowX + (arrowWidth / 2), arrowY + arrowHeight);
            dc.SetBrush(*wxBLACK_BRUSH); 
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.DrawPolygon(3, points);
        }
    }
}

void FlatUIButtonBar::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();

    for (auto& button : m_buttons) {
        if (button.rect.Contains(pos)) {
            if (button.menu) {
                wxPoint menuPos = ClientToScreen(wxPoint(button.rect.GetLeft(), button.rect.GetBottom()));
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