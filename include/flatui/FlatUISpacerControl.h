#ifndef FLATUI_SPACER_CONTROL_H
#define FLATUI_SPACER_CONTROL_H

#include <wx/wx.h>
class FlatUISpacerControl : public wxPanel
{
public:
    FlatUISpacerControl(wxWindow* parent, int width = 10, wxWindowID id = wxID_ANY);

    virtual ~FlatUISpacerControl();

    void SetSpacerWidth(int width);

    int GetSpacerWidth() const { return m_width; }

    void SetDrawSeparator(bool draw) { m_drawSeparator = draw; Refresh(); }

    bool GetDrawSeparator() const { return m_drawSeparator; }
    
    void SetAutoExpand(bool autoExpand) { m_autoExpand = autoExpand; }
    
    bool GetAutoExpand() const { return m_autoExpand; }
    
    int CalculateAutoWidth(int availableWidth) const;

protected:

    void OnPaint(wxPaintEvent& evt);
    
private:
    int m_width;            
    bool m_drawSeparator;   
    bool m_autoExpand;   
};

#endif // FLATUI_SPACER_CONTROL_H 