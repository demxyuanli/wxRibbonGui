#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIBar.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>

FlatUIPage::FlatUIPage(wxWindow* parent, const wxString& label)
    : wxControl(parent, wxID_ANY), m_label(label), m_isActive(false)
{
    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);

#ifdef __WXMSW__
    HWND hwnd = (HWND)GetHandle();
    if (hwnd) {
        long exStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_COMPOSITED);
    }
#endif

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

    m_sizer = new wxBoxSizer(wxHORIZONTAL);
    SetSizer(m_sizer);

    FlatUIEventManager::getInstance().bindPageEvents(this);

    Bind(wxEVT_PAINT, &FlatUIPage::OnPaint, this);
    Bind(wxEVT_SIZE, &FlatUIPage::OnSize, this);

    LOG_INF("Created page: " + label.ToStdString(), "FlatUIPage");
}

FlatUIPage::~FlatUIPage()
{
    for (auto panel : m_panels)
        delete panel;
}

void FlatUIPage::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);

    dc.SetBackground(wxColour(245, 245, 245));
    dc.Clear();

    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());

    dc.SetTextForeground(*wxBLACK);
    dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.DrawText(GetLabel(), 10, 5);

    LOG_DBG("Page painted: " + GetLabel().ToStdString() +
        ", Size: (" + std::to_string(GetSize().GetWidth()) +
        ", " + std::to_string(GetSize().GetHeight()) + ")",
        "FlatUIPage::OnPaint");

    evt.Skip();
}

void FlatUIPage::OnSize(wxSizeEvent& evt)
{
    wxSize newSize = evt.GetSize();

    LOG_DBG("Page resized: " + GetLabel().ToStdString() +
        ", New Size: (" + std::to_string(newSize.GetWidth()) +
        ", " + std::to_string(newSize.GetHeight()) + ")",
        "FlatUIPage::OnSize");

    if (m_sizer) {
        RecalculatePageHeight();
        Layout();
    }

    Refresh(false);
    evt.Skip();
}

void FlatUIPage::RecalculatePageHeight()
{
    static bool isRecalculating = false;
    if (isRecalculating)
        return;

    isRecalculating = true;

    if (m_panels.empty()) {
        SetMinSize(wxSize(100, 100));
        if (m_sizer) {
            m_sizer->SetDimension(0, 0, 100, 100);
        }
        isRecalculating = false;
        return;
    }

    Freeze();

    int maxHeight = 0;
    int totalWidth = 0;
    int spacing = 5;

    for (auto panel : m_panels) {
        if (!panel || !panel->IsShown()) continue;

        wxSize panelBestSize = panel->GetBestSize();
        totalWidth += panelBestSize.GetWidth() + spacing;
        maxHeight = wxMax(maxHeight, panelBestSize.GetHeight());
    }

    if (totalWidth > 0) {
        totalWidth -= spacing;
    }

    totalWidth += 10;
    maxHeight += 20;

    wxSize newMinSize(totalWidth, maxHeight);
    SetMinSize(newMinSize);

    if (m_sizer) {
        m_sizer->SetDimension(0, 0, totalWidth, maxHeight);
    }

    LOG_INF("RecalculatePageHeight: Calculated height: " + std::to_string(maxHeight) +
        ", Total width: " + std::to_string(totalWidth), "FlatUIPage");

    wxWindow* parent = GetParent();
    if (parent) {
        parent->Layout();
    }

    Thaw();
    isRecalculating = false;
}

void FlatUIPage::AddPanel(FlatUIPanel* panel)
{
    Freeze();
    m_panels.push_back(panel);

    wxBoxSizer* boxSizer = dynamic_cast<wxBoxSizer*>(GetSizer());
    if (!boxSizer) {
        boxSizer = new wxBoxSizer(wxHORIZONTAL);
        SetSizer(boxSizer);
    }

    wxSize minSize = panel->GetBestSize();
    panel->SetMinSize(minSize);

    boxSizer->Add(panel, 0, wxALL, 5);

    wxSize pageSizeForLog = GetSize();
    wxSize panelSizeForLog = panel->GetSize();

    LOG_INF("Added panel: " + panel->GetLabel().ToStdString() +
        " to page: " + GetLabel().ToStdString() +
        ". Page Size: (" + std::to_string(pageSizeForLog.GetWidth()) +
        ", " + std::to_string(pageSizeForLog.GetHeight()) + ")" +
        ". Panel Size: (" + std::to_string(panelSizeForLog.GetWidth()) +
        ", " + std::to_string(panelSizeForLog.GetHeight()) + ")",
        "FlatUIPage");

    panel->Show();
    panel->Layout();

    RecalculatePageHeight();

    Layout();
    Refresh(false);

    Thaw();
}