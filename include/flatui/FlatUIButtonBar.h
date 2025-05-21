#ifndef FLATUIBUTTONBAR_H
#define FLATUIBUTTONBAR_H

#include <wx/wx.h>
#include <wx/artprov.h> // For wxArtProvider
#include <wx/vector.h>
#include <string>
#include <wx/menu.h> // Included for wxMenu*
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC

// Forward declaration
class FlatUIPanel;

class FlatUIButtonBar : public wxControl
{
public:
    FlatUIButtonBar(FlatUIPanel* parent);
    virtual ~FlatUIButtonBar();

    void AddButton(int id, const wxString& label, const wxBitmap& bitmap, wxMenu* menu = nullptr);
    size_t GetButtonCount() const { return m_buttons.size(); }

    void OnPaint(wxPaintEvent& evt);
    void OnMouseDown(wxMouseEvent& evt);

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
};

#endif // FLATUIBUTTONBAR_H 