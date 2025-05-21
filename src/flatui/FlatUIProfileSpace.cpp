#include "flatui/FlatUIProfileSpace.h"

FlatUIProfileSpace::FlatUIProfileSpace(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxBORDER_NONE),
      m_childControl(nullptr),
      m_spaceWidth(DEFAULT_WIDTH)
{
    SetMinSize(wxSize(m_spaceWidth, wxDefaultCoord));
    Bind(wxEVT_SIZE, &FlatUIProfileSpace::OnSize, this);
    // Bind(wxEVT_PAINT, &FlatUIProfileSpace::OnPaint, this); // If custom painting needed
}

FlatUIProfileSpace::~FlatUIProfileSpace() {}

void FlatUIProfileSpace::SetChildControl(wxWindow* child)
{
    if (m_childControl && m_childControl != child) {
        m_childControl->Hide();
        m_childControl->Reparent(nullptr);
    }
    m_childControl = child;
    if (m_childControl) {
        if (m_childControl->GetParent() != this) {
            m_childControl->Reparent(this);
        }
        m_childControl->SetPosition(wxPoint(0,0));
        m_childControl->SetSize(GetClientSize());
        m_childControl->Show();
    }
    Layout();
    Refresh();
}

void FlatUIProfileSpace::SetSpaceWidth(int width)
{
    if (width > 0) {
        m_spaceWidth = width;
        SetMinSize(wxSize(m_spaceWidth, GetMinSize().y));
        if (GetParent()) {
            GetParent()->Layout();
        }
    }
}

int FlatUIProfileSpace::GetSpaceWidth() const
{
    return m_spaceWidth;
}

void FlatUIProfileSpace::OnSize(wxSizeEvent& evt)
{
    if (m_childControl) {
        m_childControl->SetSize(evt.GetSize());
    }
    evt.Skip();
}

/*
void FlatUIProfileSpace::OnPaint(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    // Custom painting if desired (e.g., background, border)
}
*/ 