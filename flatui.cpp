#include "flatui/flatui.h"



FlatUIPage::FlatUIPage(FlatUIBar* parent, const wxString& label)
    : wxControl(parent, wxID_ANY), m_label(label)
{
}


FlatUIPage::~FlatUIPage()
{
    for (auto panel : m_panels)
        delete panel;
}

void FlatUIPage::AddPanel(FlatUIPanel* panel)
{
    m_panels.push_back(panel);
    panel->SetPosition(wxPoint(0, 0));
    wxSize panelSizeForLog = GetSize(); 
    panel->SetSize(wxSize(panelSizeForLog.GetWidth(), panelSizeForLog.GetHeight()));
    Logger::getLogger().Log(Logger::LogLevel::INF, "Added panel: " + panel->GetLabel().ToStdString() + " to page: " + GetLabel().ToStdString() + ". Initial Position: (0, 0), Initial Size: (" + std::to_string(panelSizeForLog.GetWidth()) + ", " + std::to_string(panelSizeForLog.GetHeight()) + ")", "FlatUIPage");

    // Create a sizer and add the panel to it
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(panel, 1, wxEXPAND | wxALL, 5);
    SetSizer(sizer);
}

FlatUIPanel::FlatUIPanel(FlatUIPage* parent, const wxString& label, int orientation)
    : wxControl(parent, wxID_ANY), m_label(label), m_orientation(orientation)
{
    Bind(wxEVT_PAINT, &FlatUIPanel::OnPaint, this);
    m_sizer = new wxBoxSizer(m_orientation);
    SetSizer(m_sizer);
}

FlatUIPanel::~FlatUIPanel()
{
    for (auto buttonBar : m_buttonBars)
        delete buttonBar;
    for (auto gallery : m_galleries)
        delete gallery;
}

void FlatUIPanel::AddButtonBar(FlatUIButtonBar* buttonBar, int proportion, int flag, int border)
{
    m_buttonBars.push_back(buttonBar);
    // Logger call can be kept if general addition logging is desired, but position/size is now sizer-managed.
    // Logger::getLogger().Log(Logger::LogLevel::INF, "Added ButtonBar to panel: " + GetLabel().ToStdString(), "FlatUIPanel");

    if (m_sizer)
    {
        m_sizer->Add(buttonBar, proportion, flag, border);
        // No longer call SetPosition or SetSize directly for buttonBar
    }
    // Refresh(); // Refresh of the panel might still be needed or Layout if sizer changes things.
}

void FlatUIPanel::AddGallery(FlatUIGallery* gallery, int proportion, int flag, int border)
{
    m_galleries.push_back(gallery);
    // Logger::getLogger().Log(Logger::LogLevel::INF, "Added Gallery to panel: " + GetLabel().ToStdString(), "FlatUIPanel");

    if (m_sizer)
    {
        m_sizer->Add(gallery, proportion, flag, border);
        // No longer call SetPosition or SetSize directly for gallery
    }
    // Refresh();
}

void FlatUIPanel::OnPaint(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    wxSize size = GetSize();
    dc.SetBackground(wxColour(240, 240, 240));
    dc.Clear();

    // Set pen for black border
    dc.SetPen(wxPen(wxColour(0, 0, 0), 1));
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
}



FlatUIGallery::FlatUIGallery(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY)
{
    Bind(wxEVT_PAINT, &FlatUIGallery::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FlatUIGallery::OnMouseDown, this);
}

FlatUIGallery::~FlatUIGallery()
{
}

void FlatUIGallery::AddItem(const wxBitmap& bitmap, int id)
{
    ItemInfo info;
    info.bitmap = bitmap;
    info.id = id;
    m_items.push_back(info);
    Refresh();
}

void FlatUIGallery::OnPaint(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    wxSize size = GetSize();
    dc.SetBackground(wxColour(230, 230, 230));
    dc.Clear();

    int x = 10;
    int y = 10;
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        ItemInfo& info = m_items[i];
        dc.DrawBitmap(info.bitmap, x, y);
        info.rect = wxRect(x, y, info.bitmap.GetWidth(), info.bitmap.GetHeight());
        x += info.bitmap.GetWidth() + 10;
    }
}

void FlatUIGallery::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    Logger::getLogger().Log(Logger::LogLevel::INF, "Mouse down event in FlatUIGallery at position: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")", "FlatUIGallery");
    for (const auto& item : m_items)
    {
        if (item.rect.Contains(pos))
        {
            Logger::getLogger().Log(Logger::LogLevel::INF, "Clicked on item with ID: " + std::to_string(item.id), "FlatUIGallery");
            wxCommandEvent event(wxEVT_BUTTON, item.id);
            event.SetEventObject(this);
            ProcessWindowEvent(event);
            break;
        }
    }
    evt.Skip();
}