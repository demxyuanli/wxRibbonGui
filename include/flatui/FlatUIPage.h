#ifndef FLATUIPAGE_H
#define FLATUIPAGE_H

#include <wx/wx.h>
#include <wx/vector.h>
#include <string>

// Forward declarations
class FlatUIBar;
class FlatUIPanel;

class FlatUIPage : public wxControl
{
public:
    // Constructor takes a FlatUIBar* parent
    FlatUIPage(FlatUIBar* parent, const wxString& label);
    virtual ~FlatUIPage();

    void AddPanel(FlatUIPanel* panel);

    wxString GetLabel() const { return m_label; }
    
    wxVector<FlatUIPanel*>& GetPanels() { return m_panels; }
    const wxVector<FlatUIPanel*>& GetPanels() const { return m_panels; }

    void OnPaint(wxPaintEvent& evt);
    void OnSize(wxSizeEvent& evt);

private:
    wxString m_label;
    wxVector<FlatUIPanel*> m_panels;
    // wxBoxSizer* m_sizer; // If sizer is part of FlatUIPage, declare here. Moved from cpp for clarity.
                        // If it was only in AddPanel, it might not need to be a member.
                        // Keeping it out unless it's confirmed to be a persistent member.
};

#endif // FLATUIPAGE_H 