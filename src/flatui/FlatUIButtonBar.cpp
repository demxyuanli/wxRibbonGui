#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIEventManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>

FlatUIButtonBar::FlatUIButtonBar(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(100, 40));
    Bind(wxEVT_PAINT, &FlatUIButtonBar::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FlatUIButtonBar::OnMouseDown, this);
    Bind(wxEVT_SIZE, &FlatUIButtonBar::OnSize, this);
}

FlatUIButtonBar::~FlatUIButtonBar()
{
}

void FlatUIButtonBar::AddButton(int id, const wxString& label, const wxBitmap& bitmap, wxMenu* menu)
{
    Freeze();
    ButtonInfo button;
    button.id = id;
    button.label = label;
    button.icon = bitmap;
    button.menu = menu;
    button.isDropDown = (menu != nullptr);

    int buttonWidth = 80;
    int buttonHeight = 30;

    if (bitmap.IsOk()) {
        buttonWidth = bitmap.GetWidth() + 10;
        if (!label.empty()) {
            wxClientDC dc(this);
            wxSize textSize = dc.GetTextExtent(label);
            buttonWidth += textSize.GetWidth() + 10;
        }
    }

    int x = 10;
    if (!m_buttons.empty()) {
        x = m_buttons.back().rect.GetRight() + 10;
    }

    button.rect = wxRect(x, 5, buttonWidth, buttonHeight);
    m_buttons.push_back(button);

    int totalWidth = x + buttonWidth + 10;
    SetMinSize(wxSize(totalWidth, 40));

    wxWindow* parent = GetParent();
    if (parent) {
        FlatUIPanel* panel = dynamic_cast<FlatUIPanel*>(parent);
        if (panel) {
            panel->UpdatePanelSize();
        }
    }

    Refresh();
    Thaw();

    LOG_INF("Added button " + label.ToStdString() + " to ButtonBar, new size: " +
        std::to_string(totalWidth) + "x40", "FlatUIButtonBar");
}

void FlatUIButtonBar::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetSize();

    dc.SetBrush(wxColour(220, 230, 250));
    dc.SetPen(wxPen(wxColour(100, 120, 180), 1));
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    if (m_buttons.empty()) {
        dc.SetTextForeground(wxColour(80, 80, 80));
        wxString text = "ButtonBar";
        wxSize textSize = dc.GetTextExtent(text);
        int x = (size.GetWidth() - textSize.GetWidth()) / 2;
        int y = (size.GetHeight() - textSize.GetHeight()) / 2;
        dc.DrawText(text, x, y);
    }

    for (auto& button : m_buttons) {
        wxRect buttonRect = button.rect;

        if (button.hovered) {
            dc.SetBrush(wxColour(180, 200, 240));
            dc.SetPen(wxPen(wxColour(70, 90, 140), 1));
        }
        else {
            dc.SetBrush(wxColour(200, 220, 240));
            dc.SetPen(wxPen(wxColour(100, 120, 180), 1));
        }

        dc.DrawRoundedRectangle(buttonRect, 3);

        if (button.icon.IsOk()) {
            int iconX = buttonRect.GetLeft() + 5;
            int iconY = buttonRect.GetTop() + (buttonRect.GetHeight() - button.icon.GetHeight()) / 2;
            dc.DrawBitmap(button.icon, iconX, iconY, true);
        }
        if (!button.label.empty()) {
            dc.SetTextForeground(wxColour(40, 40, 80));
            wxSize textSize = dc.GetTextExtent(button.label);

            int textX = button.icon.IsOk() ? buttonRect.GetLeft() + button.icon.GetWidth() + 10 : buttonRect.GetLeft() + 5;
            int textY = buttonRect.GetTop() + (buttonRect.GetHeight() - textSize.GetHeight()) / 2;
            dc.DrawText(button.label, textX, textY);
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