#include "flatui/FlatUIButtonBar.h"
#include "flatui/FlatUIPanel.h" // Forward declare
#include "flatui/FlatUIEventManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC

FlatUIButtonBar::FlatUIButtonBar(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY)
{
    // Required for wxAutoBufferedPaintDC
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    // Set a minimum size to ensure the control has reasonable dimensions
    SetMinSize(wxSize(100, 40));
}

FlatUIButtonBar::~FlatUIButtonBar()
{
}

void FlatUIButtonBar::AddButton(int id, const wxString& label, const wxBitmap& bitmap, wxMenu* menu)
{
    ButtonInfo button;
    button.id = id;
    button.label = label;
    button.icon = bitmap; 
    button.menu = menu;
    button.isDropDown = (menu != nullptr);
    
    // Calculate button dimensions
    int buttonWidth = 80;  // Default width
    int buttonHeight = 30; // Default height

    if (bitmap.IsOk()) {
        buttonWidth = bitmap.GetWidth() + 10;  
        if (!label.empty()) {
            wxClientDC dc(this);
            wxSize textSize = dc.GetTextExtent(label);
            buttonWidth += textSize.GetWidth() + 10;  
        }
    }
    
    // Calculate button position
    int x = 10;  // Initial margin
    if (!m_buttons.empty()) {
        const ButtonInfo& lastButton = m_buttons.back();
        x = lastButton.rect.GetRight() + 10;
        
        // Check if we're exceeding the control width - adjust positions if needed
        wxSize currentSize = GetSize();
        if (currentSize.GetWidth() > 0 && x + buttonWidth > currentSize.GetWidth() - 10) {
            // 如果超出了当前控件宽度，记录警告并拒绝添加超出控件的按钮
            Logger::getLogger().Log(Logger::LogLevel::WRN, 
                "Button '" + label.ToStdString() + "' position exceeds ButtonBar width", 
                "FlatUIButtonBar");
            
            // 返回不添加按钮，防止按钮出现在第二行
            return;
        }
    }
    
    // Set the button rectangle
    button.rect = wxRect(x, 10, buttonWidth, buttonHeight);
    m_buttons.push_back(button);
    
    // Update the control's size to accommodate all buttons
    wxSize minSize = GetMinSize();
    int requiredWidth = x + buttonWidth + 10; // Last button position + width + margin
    if (requiredWidth > minSize.GetWidth()) {
        SetMinSize(wxSize(requiredWidth, minSize.GetHeight()));
        InvalidateBestSize();
    }
    
    Refresh();
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
    
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        ButtonInfo& button = m_buttons[i];
        wxRect buttonRect = button.rect;
        
        if (button.hovered) {
            dc.SetBrush(wxColour(180, 200, 240)); 
            dc.SetPen(wxPen(wxColour(70, 90, 140), 1)); 
        } else {
            dc.SetBrush(wxColour(200, 220, 240)); 
            dc.SetPen(wxPen(wxColour(100, 120, 180), 1)); 
        }
        
        dc.DrawRoundedRectangle(buttonRect, 3);
        
        if (button.icon.IsOk())
        {
            int iconX = buttonRect.GetLeft() + 5;
            int iconY = buttonRect.GetTop() + (buttonRect.GetHeight() - button.icon.GetHeight()) / 2;
            dc.DrawBitmap(button.icon, iconX, iconY, true);
        }
        if (!button.label.empty())
        {
            dc.SetTextForeground(wxColour(40, 40, 80)); 
            wxSize textSize = dc.GetTextExtent(button.label);
            
            int textX;
            if (button.icon.IsOk()) {
                textX = buttonRect.GetLeft() + button.icon.GetWidth() + 10;
            } else {
                textX = buttonRect.GetLeft() + 5;
            }
            
            int textY = buttonRect.GetTop() + (buttonRect.GetHeight() - textSize.GetHeight()) / 2;
            dc.DrawText(button.label, textX, textY);
        }
    }
}

void FlatUIButtonBar::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        ButtonInfo& button = m_buttons[i];
        if (button.rect.Contains(pos))
        {
            if (button.menu)
            {
                wxPoint menuPos = ClientToScreen(wxPoint(button.rect.GetLeft(), button.rect.GetBottom()));
                PopupMenu(button.menu, menuPos.x, menuPos.y);
            }
            else
            {
                wxCommandEvent event(wxEVT_BUTTON, button.id);
                event.SetEventObject(this);
                GetParent()->ProcessWindowEvent(event);
            }
            break;
        }
    }
    
    evt.Skip();
}