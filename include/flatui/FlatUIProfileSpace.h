#ifndef FLATUI_PROFILE_SPACE_H
#define FLATUI_PROFILE_SPACE_H

#include <wx/wx.h>

class FlatUIProfileSpace : public wxPanel // Inherit from wxPanel
{
public:
    FlatUIProfileSpace(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~FlatUIProfileSpace();

    void SetChildControl(wxWindow* child);
    wxWindow* GetChildControl() const { return m_childControl; }

    void SetSpaceWidth(int width);
    int GetSpaceWidth() const;

protected:
    void OnSize(wxSizeEvent& evt);
    // void OnPaint(wxPaintEvent& evt); // Optional

private:
    wxWindow* m_childControl;
    int m_spaceWidth;

    static const int DEFAULT_WIDTH = 100; // Default width for profile space
};

#endif // FLATUI_PROFILE_SPACE_H 