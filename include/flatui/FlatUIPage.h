#ifndef FLATUIPAGE_H
#define FLATUIPAGE_H

#include <wx/wx.h>
#include <wx/vector.h>
#include <string>

// Forward declarations
class FlatUIBar;
class FlatUIPanel;

class FlatUIPage : public wxWindow
{
public:
    // Constructor takes a wxWindow* parent (更通用的父窗口类型)
    FlatUIPage(wxWindow* parent, const wxString& label);
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
    wxBoxSizer* m_sizer;  // 添加sizer成员变量用于页面布局
};

#endif // FLATUIPAGE_H 