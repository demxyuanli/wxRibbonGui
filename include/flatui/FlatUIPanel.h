#ifndef FLATUIPANEL_H
#define FLATUIPANEL_H

#include <wx/wx.h>
#include <wx/vector.h>
#include <string>


// Forward declarations
class FlatUIPage;
class FlatUIButtonBar;
class FlatUIGallery;
// wxBoxSizer is a wxWidgets class, typically available through <wx/wx.h> or <wx/sizer.h>
// If wxBoxSizer* m_sizer; causes issues, ensure <wx/sizer.h> is included or wx/wx.h is sufficient.

class FlatUIPanel : public wxControl
{
public:
    FlatUIPanel(FlatUIPage* parent, const wxString& label, int orientation = wxVERTICAL);
    virtual ~FlatUIPanel();

    void AddButtonBar(FlatUIButtonBar* buttonBar, int proportion = 0, int flag = wxEXPAND | wxALL, int border = 5);
    void AddGallery(FlatUIGallery* gallery, int proportion = 0, int flag = wxEXPAND | wxALL, int border = 5);
    wxString GetLabel() const { return m_label; }

    void OnPaint(wxPaintEvent& evt); // Keep if specific OnPaint is needed, otherwise wxControl default

private:
    wxString m_label;
    wxVector<FlatUIButtonBar*> m_buttonBars;
    wxVector<FlatUIGallery*> m_galleries;
    wxBoxSizer* m_sizer; // This was confirmed as a member
    int m_orientation;
};

#endif // FLATUIPANEL_H 