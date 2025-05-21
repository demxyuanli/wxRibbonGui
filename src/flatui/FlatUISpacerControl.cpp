#include "flatui/FlatUISpacerControl.h"
#include <wx/dcbuffer.h>

FlatUISpacerControl::FlatUISpacerControl(wxWindow* parent, int width, wxWindowID id)
    : wxPanel(parent, id), m_width(width), m_drawSeparator(false), m_autoExpand(false)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    SetMinSize(wxSize(m_width, -1));
    SetSize(wxSize(m_width, -1));
    
    Bind(wxEVT_PAINT, &FlatUISpacerControl::OnPaint, this);
}

FlatUISpacerControl::~FlatUISpacerControl()
{
    Unbind(wxEVT_PAINT, &FlatUISpacerControl::OnPaint, this);
}

void FlatUISpacerControl::SetSpacerWidth(int width)
{
    if (width >= 0 && width != m_width)
    {
        m_width = width;
        if (!m_autoExpand)
        {
            SetMinSize(wxSize(m_width, -1));
            SetSize(wxSize(m_width, -1));
        }
        
        wxWindow* parent = GetParent();
        if (parent)
        {
            parent->Layout();
        }
        Refresh();
    }
}

int FlatUISpacerControl::CalculateAutoWidth(int availableWidth) const
{
    if (!m_autoExpand)
    {
        return m_width;
    }
    return wxMax(m_width, availableWidth);
}

void FlatUISpacerControl::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetSize();
    
    wxColour bgColor = GetParent() ? GetParent()->GetBackgroundColour() : wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR);
    dc.SetBackground(bgColor);
    dc.Clear();
    
    if (m_drawSeparator)
    {
        dc.SetPen(wxPen(wxColour(200, 200, 200), 1));
        dc.DrawLine(size.GetWidth() / 2, 2, size.GetWidth() / 2, size.GetHeight() - 2);
    }
} 