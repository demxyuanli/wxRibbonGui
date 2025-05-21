#ifndef FLATUI_HOME_SPACE_H
#define FLATUI_HOME_SPACE_H

#include <wx/wx.h>

class FlatUIHomeSpace : public wxControl
{
public:
    FlatUIHomeSpace(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~FlatUIHomeSpace();

    void SetMenu(wxMenu* menu);
    void SetIcon(const wxBitmap& icon);
    void SetButtonWidth(int width) { m_buttonWidth = width; Refresh(); }
    int GetButtonWidth() const { return m_buttonWidth; }

    void OnPaint(wxPaintEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);
    void OnMouseMove(wxMouseEvent& evt);
    void OnMouseLeave(wxMouseEvent& evt);

    void CalculateButtonRect(const wxSize& controlSize);

private:
    wxMenu* m_menu;         // The dropdown menu, owned by caller
    wxBitmap m_icon;        // Icon for the button
    wxRect m_buttonRect;    // Clickable area, calculated in OnPaint or OnSize
    bool m_hover;         // True if mouse is over the button
    int m_buttonWidth;    // Width of the home button area
    static const int DEFAULT_BUTTON_WIDTH = 30;
};

#endif // FLATUI_HOME_SPACE_H 