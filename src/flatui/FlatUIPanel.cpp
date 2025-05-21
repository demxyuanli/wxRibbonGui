#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIPage.h" // Forward declare
#include "flatui/FlatUIButtonBar.h" // Forward declare
#include "flatui/FlatUIGallery.h" // Forward declare
#include "flatui/FlatUIEventManager.h" 
#include "logger/Logger.h"
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC


FlatUIPanel::FlatUIPanel(FlatUIPage* parent, const wxString& label, int orientation)
    : wxControl(parent, wxID_ANY), m_label(label), m_orientation(orientation)
{
    // 启用双缓冲绘图以减少闪烁
    SetDoubleBuffered(true);
    
    // 在Windows平台上使用WS_EX_COMPOSITED风格减少闪烁
#ifdef __WXMSW__
    HWND hwnd = (HWND)GetHandle();
    if (hwnd) {
        long exStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_COMPOSITED);
    }
#endif
    
    // Required for wxAutoBufferedPaintDC
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
    m_sizer = new wxBoxSizer(m_orientation);
    SetSizer(m_sizer);
    
    FlatUIEventManager::getInstance().bindPanelEvents(this);
    
    FlatUIEventManager::getInstance().bindSizeEvents(this, [this](wxSizeEvent& event) {
        if (m_sizer) {
            m_sizer->SetDimension(0, 0, event.GetSize().GetWidth(), event.GetSize().GetHeight());
            Logger::getLogger().Log(Logger::LogLevel::INF, "Panel resized: " + GetLabel().ToStdString() + 
                                   " Size: (" + std::to_string(event.GetSize().GetWidth()) + ", " + 
                                   std::to_string(event.GetSize().GetHeight()) + ")", "FlatUIPanel");
        }
        event.Skip();
    });
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
    
    // 为ButtonBar设置最小尺寸，防止在Panel隐藏时ButtonBar尺寸为零
    wxSize minSize(150, 30);
    buttonBar->SetMinSize(minSize);
    
    // Log the addition
    Logger::getLogger().Log(Logger::LogLevel::INF, "Added ButtonBar to panel: " + GetLabel().ToStdString(), "FlatUIPanel");

    // Bind events to the new button bar
    FlatUIEventManager::getInstance().bindButtonBarEvents(buttonBar);

    // If no specific flags are provided, use default flags for good layout
    if (flag == 0) {
        flag = wxEXPAND | wxALL;
    }
    
    flag |= wxALIGN_LEFT; 
    
    // If no specific border is provided, use a default spacing
    if (border == 0) {
        border = 5;
    }

    // Add to sizer with appropriate flags
    if (m_sizer)
    {
        m_sizer->Add(buttonBar, proportion, flag, border);
        
        // 强制执行布局，即使Panel还未显示
        m_sizer->Layout();
        Layout();
        
        // Force parent layout update as well
        wxWindow* parent = GetParent();
        if (parent) {
            parent->Layout();
        }
    }
    
    // 确保ButtonBar可见
    buttonBar->Show();
    Refresh();
}

void FlatUIPanel::AddGallery(FlatUIGallery* gallery, int proportion, int flag, int border)
{
    m_galleries.push_back(gallery);
    
    // 为Gallery设置最小尺寸，防止在Panel隐藏时Gallery尺寸为零
    wxSize minSize(200, 80);
    gallery->SetMinSize(minSize);
    
    Logger::getLogger().Log(Logger::LogLevel::INF, "Added Gallery to panel: " + GetLabel().ToStdString(), "FlatUIPanel");

    FlatUIEventManager::getInstance().bindGalleryEvents(gallery);

    if (m_sizer)
    {
        m_sizer->Add(gallery, proportion, flag, border);
        
        // 强制执行布局，即使Panel还未显示
        m_sizer->Layout();
        Layout(); 
        
        // 更新父窗口布局
        wxWindow* parent = GetParent();
        if (parent) {
            parent->Layout();
        }
    }
    
    // 确保Gallery可见
    gallery->Show();
    Refresh(); 
}

void FlatUIPanel::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetSize();
    
    // Set a light gray background (slightly darker than page)
    dc.SetBackground(wxColour(240, 240, 240));
    dc.Clear();

    // Set pen for black border
    dc.SetPen(wxPen(*wxBLACK, 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
    
    // Draw panel label in top-left with black font
    if (!m_label.IsEmpty()) {
        dc.SetTextForeground(*wxBLACK);
        dc.SetFont(wxFont(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
        dc.DrawText(m_label, 5, 2);
    }
}
