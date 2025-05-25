#ifndef FLATUIBUTTONBAR_H
#define FLATUIBUTTONBAR_H

#include <wx/wx.h>
#include <wx/artprov.h> // For wxArtProvider
#include <wx/vector.h>
#include <string>
#include <wx/menu.h> // Included for wxMenu*
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC
#include "logger/Logger.h"
#include "flatui/FlatUIConstants.h"

// Forward declaration
class FlatUIPanel;

// Enum for button display styles
enum class ButtonDisplayStyle {
    ICON_ONLY,        // Only icon is displayed
    TEXT_ONLY,        // Only text is displayed
    ICON_TEXT_BESIDE, // Icon on the left, text on the right (default)
    ICON_TEXT_BELOW   // Icon on top, text below
};

class FlatUIButtonBar : public wxControl
{
public:
    // Button Style
    enum class ButtonStyle {
        DEFAULT,        // Default flat style
        RAISED,         // Raised button appearance
        OUTLINED,       // Outlined button style
        GHOST,          // Ghost button (no background)
        PILL            // Pill-shaped button
    };
    
    // Button Border Style
    enum class ButtonBorderStyle {
        SOLID,          // Solid line border
        DASHED,         // Dashed line border
        DOTTED,         // Dotted line border
        DOUBLE,         // Double line border
        ROUNDED         // Rounded corners
    };

    FlatUIButtonBar(FlatUIPanel* parent);
    virtual ~FlatUIButtonBar();

    void AddButton(int id, const wxString& label, const wxBitmap& bitmap = wxNullBitmap, wxMenu* menu = nullptr);
    size_t GetButtonCount() const { return m_buttons.size(); }

    void SetDisplayStyle(ButtonDisplayStyle styleh);
    ButtonDisplayStyle GetDisplayStyle() const { return m_displayStyle; }

    // Button spacing and padding
    void SetBarBorderColour(const wxColour& colour);
    wxColour GetBarBorderColour() const { return m_barBorderColour; }

    // Button spacing and padding
    void SetBarBorderWidth(int width);
    int GetBarBorderWidth() const { return m_barBorderWidth; }

    // Style configuration methods
    void SetButtonStyle(ButtonStyle style);
    ButtonStyle GetButtonStyle() const { return m_buttonStyle; }
    
    void SetButtonBorderStyle(ButtonBorderStyle style);
    ButtonBorderStyle GetButtonBorderStyle() const { return m_buttonBorderStyle; }
    
    // Button appearance configuration
    void SetButtonBackgroundColour(const wxColour& colour);
    wxColour GetButtonBackgroundColour() const { return m_buttonBgColour; }
    
    void SetButtonHoverBackgroundColour(const wxColour& colour);
    wxColour GetButtonHoverBackgroundColour() const { return m_buttonHoverBgColour; }
    
    void SetButtonPressedBackgroundColour(const wxColour& colour);
    wxColour GetButtonPressedBackgroundColour() const { return m_buttonPressedBgColour; }
    
    void SetButtonTextColour(const wxColour& colour);
    wxColour GetButtonTextColour() const { return m_buttonTextColour; }
    
    void SetButtonBorderColour(const wxColour& colour);
    wxColour GetButtonBorderColour() const { return m_buttonBorderColour; }
    
    void SetButtonBorderWidth(int width);
    int GetButtonBorderWidth() const { return m_buttonBorderWidth; }
    
    void SetButtonCornerRadius(int radius);
    int GetButtonCornerRadius() const { return m_buttonCornerRadius; }
    
    // Button spacing and padding
    void SetButtonSpacing(int spacing);
    int GetButtonSpacing() const { return m_buttonSpacing; }
    
    void SetButtonPadding(int horizontal, int vertical);
    void GetButtonPadding(int& horizontal, int& vertical) const;
    
    // Bar background
    void SetBarBackgroundColour(const wxColour& colour);
    wxColour GetBarBackgroundColour() const { return m_barBgColour; }
    
    // Enable/disable hover effects
    void SetHoverEffectsEnabled(bool enabled);
    bool GetHoverEffectsEnabled() const { return m_hoverEffectsEnabled; }

    void OnPaint(wxPaintEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnMouseLeave(wxMouseEvent& evt);
    void OnSize(wxSizeEvent& evt);

protected:
    virtual wxSize DoGetBestSize() const override;

private:
    struct ButtonInfo
    {
        int id;
        wxString label;
        wxBitmap icon;
        wxRect rect;
        wxMenu* menu = nullptr;
        bool isDropDown = false;
        bool hovered = false;
        bool pressed = false;
    };
    wxVector<ButtonInfo> m_buttons;
    ButtonDisplayStyle m_displayStyle;
    
    // Style members
    ButtonStyle m_buttonStyle;
    ButtonBorderStyle m_buttonBorderStyle;
    wxColour m_buttonBgColour;
    wxColour m_buttonHoverBgColour;
    wxColour m_buttonPressedBgColour;
    wxColour m_buttonTextColour;
    wxColour m_buttonBorderColour;
    wxColour m_barBgColour;
    wxColour m_barBorderColour;
    int m_buttonBorderWidth;
    int m_buttonCornerRadius;
    int m_buttonSpacing;
    int m_buttonHorizontalPadding;
    int m_buttonVerticalPadding;
    int m_barBorderWidth;
    bool m_hoverEffectsEnabled;
    int m_hoveredButtonIndex; // Track which button is hovered (-1 for none)
    
    void RecalculateLayout(); // Helper for layout updates
    void DrawButton(wxDC& dc, const ButtonInfo& button, int index);
    void DrawButtonBackground(wxDC& dc, const wxRect& rect, bool isHovered, bool isPressed);
    void DrawButtonBorder(wxDC& dc, const wxRect& rect, bool isHovered, bool isPressed);
};

#endif // FLATUIBUTTONBAR_H 