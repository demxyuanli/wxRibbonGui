#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUIPanel.h" // Use forward declaration
#include "flatui/FlatUIBar.h"   // Use forward declaration
#include "logger/Logger.h"
#include <wx/dcbuffer.h>


FlatUIPage::FlatUIPage(FlatUIBar* parent, const wxString& label)
    : wxControl(parent, wxID_ANY), m_label(label)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    FlatUIEventManager::getInstance().bindPageEvents(this);
    
    Bind(wxEVT_PAINT, &FlatUIPage::OnPaint, this);
    
    Bind(wxEVT_SIZE, &FlatUIPage::OnSize, this);
    
    Logger::getLogger().Log(Logger::LogLevel::INF, 
        "Created page: " + label.ToStdString(), "FlatUIPage");
}

FlatUIPage::~FlatUIPage()
{
    for (auto panel : m_panels)
        delete panel;
}

void FlatUIPage::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    
    // Set background to a light gray
    dc.SetBackground(wxColour(245, 245, 245));
    dc.Clear();
    
    // Use black pen for the border (was light gray before)
    dc.SetPen(wxPen(*wxBLACK, 1)); // Use wxBLACK which is black, with width of 1
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
    
    // Optional: Draw the page label text at top-left corner with black font
    dc.SetTextForeground(*wxBLACK);
    dc.SetFont(wxFont(9, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD));
    dc.DrawText(GetLabel(), 10, 5);
    
    Logger::getLogger().Log(Logger::LogLevel::DBG, 
        "Page painted: " + GetLabel().ToStdString() + 
        ", Size: (" + std::to_string(GetSize().GetWidth()) + 
        ", " + std::to_string(GetSize().GetHeight()) + ")",
        "FlatUIPage::OnPaint");
        
    evt.Skip();
}

void FlatUIPage::OnSize(wxSizeEvent& evt)
{
    Logger::getLogger().Log(Logger::LogLevel::DBG, 
        "Page resized: " + GetLabel().ToStdString() + 
        ", New Size: (" + std::to_string(evt.GetSize().GetWidth()) + 
        ", " + std::to_string(evt.GetSize().GetHeight()) + ")",
        "FlatUIPage::OnSize");
    
    Layout();
    Refresh();
    
    evt.Skip();
}

void FlatUIPage::AddPanel(FlatUIPanel* panel)
{
    m_panels.push_back(panel);
    
    wxBoxSizer* sizer = static_cast<wxBoxSizer*>(GetSizer());
    if (!sizer) {
        sizer = new wxBoxSizer(wxVERTICAL);
        SetSizer(sizer);
    }
    
    sizer->Add(panel, 1, wxEXPAND | wxALL, 5);

    wxSize minSize(100, 100);
    panel->SetMinSize(minSize);

    sizer->Layout();

    wxSize panelSizeForLog = panel->GetSize(); 
    
    Logger::getLogger().Log(Logger::LogLevel::INF, 
        "Added panel: " + panel->GetLabel().ToStdString() + 
        " to page: " + GetLabel().ToStdString() + 
        ". Initial Size: (" + std::to_string(panelSizeForLog.GetWidth()) + 
        ", " + std::to_string(panelSizeForLog.GetHeight()) + ")", 
        "FlatUIPage");

    Layout();
    
    panel->Show();
    panel->Refresh();
}