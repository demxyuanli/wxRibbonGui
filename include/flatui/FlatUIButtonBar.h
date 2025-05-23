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
    FlatUIButtonBar(FlatUIPanel* parent);
    virtual ~FlatUIButtonBar();

    void AddButton(int id, const wxString& label, const wxBitmap& bitmap = wxNullBitmap, wxMenu* menu = nullptr);
    size_t GetButtonCount() const { return m_buttons.size(); }

    void SetDisplayStyle(ButtonDisplayStyle style);
    ButtonDisplayStyle GetDisplayStyle() const { return m_displayStyle; };

    void OnPaint(wxPaintEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);
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
    };
    wxVector<ButtonInfo> m_buttons;
    ButtonDisplayStyle m_displayStyle;
    void RecalculateLayout(); // Helper for layout updates
};

#endif // FLATUIBUTTONBAR_H 