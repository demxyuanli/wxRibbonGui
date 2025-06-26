#include "flatui/FlatUIFloatingWindow.h"
#include "flatui/FlatUIPage.h"
#include "logger/Logger.h"

// Define the custom event
wxDEFINE_EVENT(wxEVT_FLOATING_WINDOW_DISMISSED, wxCommandEvent);

wxBEGIN_EVENT_TABLE(FlatUIFloatingWindow, wxPopupTransientWindow)
    EVT_SIZE(FlatUIFloatingWindow::OnSize)
wxEND_EVENT_TABLE()

FlatUIFloatingWindow::FlatUIFloatingWindow(wxWindow* parent)
    : wxPopupTransientWindow(parent, wxBORDER_NONE),
      m_currentPage(nullptr)
{
    SetName("FlatUIFloatingWindow");
    
    // Create sizer for layout
    m_sizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(m_sizer);
    
    // Set background color to match the ribbon
    SetBackgroundColour(parent->GetBackgroundColour());
    
    LOG_INF("Created FlatUIFloatingWindow", "FlatUIFloatingWindow");
}

FlatUIFloatingWindow::~FlatUIFloatingWindow()
{
    if (m_currentPage) {
        m_sizer->Detach(m_currentPage);
        m_currentPage = nullptr;
    }
    LOG_INF("Destroyed FlatUIFloatingWindow", "FlatUIFloatingWindow");
}

void FlatUIFloatingWindow::SetPageContent(FlatUIPage* page)
{
    if (m_currentPage == page) {
        return; // Same page, no change needed
    }
    
    // Remove current page if any
    if (m_currentPage) {
        m_sizer->Detach(m_currentPage);
        m_currentPage->Reparent(GetParent()); // Return to original parent
        m_currentPage->Hide();
    }
    
    // Set new page
    m_currentPage = page;
    if (m_currentPage) {
        m_currentPage->Reparent(this);
        m_sizer->Add(m_currentPage, 1, wxEXPAND | wxALL, 0);
        m_currentPage->Show();
        
        // Update layout
        Layout();
        
        LOG_INF("Set page content: " + m_currentPage->GetLabel().ToStdString(), "FlatUIFloatingWindow");
    }
}

void FlatUIFloatingWindow::ShowAt(const wxPoint& position, const wxSize& size)
{
    if (!m_currentPage) {
        return;
    }
    
    wxSize windowSize = size;
    if (windowSize == wxDefaultSize) {
        // Calculate size based on page content if no explicit size is given
        wxSize pageSize = m_currentPage->GetBestSize();
        // Ensure minimum size
        pageSize.SetWidth(wxMax(pageSize.GetWidth(), 300));
        pageSize.SetHeight(wxMax(pageSize.GetHeight(), 100));
        windowSize = pageSize;
    }
    
    // Check screen boundaries and adjust position if needed
    wxSize screenSize = wxGetDisplaySize();
    wxPoint adjustedPos = position;
    
    if (adjustedPos.x + windowSize.GetWidth() > screenSize.GetWidth()) {
        adjustedPos.x = screenSize.GetWidth() - windowSize.GetWidth() - 10;
    }
    if (adjustedPos.y + windowSize.GetHeight() > screenSize.GetHeight()) {
        adjustedPos.y = screenSize.GetHeight() - windowSize.GetHeight() - 10;
    }
    
    // Set size and position
    SetSize(windowSize);
    SetPosition(adjustedPos);
    
    // Show the window
    Show(true);
    
    // Update page layout
    if (m_currentPage) {
        m_currentPage->UpdateLayout();
    }
    
    LOG_INF("Showed floating window at position (" + 
            std::to_string(adjustedPos.x) + ", " + std::to_string(adjustedPos.y) + ")", 
            "FlatUIFloatingWindow");
}

void FlatUIFloatingWindow::HideWindow()
{
    if (IsShown()) {
        Hide();
        
        // Return page to original parent
        if (m_currentPage) {
            m_sizer->Detach(m_currentPage);
            m_currentPage->Reparent(GetParent());
            m_currentPage->Hide();
            m_currentPage = nullptr;
        }
        
        // Notify parent (FlatUIBar) that floating window was hidden
        wxWindow* parent = GetParent();
        if (parent) {
            wxCommandEvent event(wxEVT_FLOATING_WINDOW_DISMISSED, GetId());
            event.SetEventObject(this);
            wxPostEvent(parent, event);
        }
        
        LOG_INF("Hidden floating window", "FlatUIFloatingWindow");
    }
}

void FlatUIFloatingWindow::OnDismiss()
{
    // This is called automatically by wxPopupTransientWindow when dismissed
    LOG_INF("FlatUIFloatingWindow dismissed by wxPopupTransientWindow", "FlatUIFloatingWindow");
    
    // Return page to original parent
    if (m_currentPage) {
        m_sizer->Detach(m_currentPage);
        m_currentPage->Reparent(GetParent());
        m_currentPage->Hide();
        m_currentPage = nullptr;
    }
    
    // Notify parent (FlatUIBar) that floating window was dismissed
    wxWindow* parent = GetParent();
    if (parent) {
        wxCommandEvent event(wxEVT_FLOATING_WINDOW_DISMISSED, GetId());
        event.SetEventObject(this);
        wxPostEvent(parent, event);
    }
}

void FlatUIFloatingWindow::OnSize(wxSizeEvent& event)
{
    Layout();
    event.Skip();
}

bool FlatUIFloatingWindow::ProcessLeftDown(wxMouseEvent& event)
{
    // Check if the click is inside the floating window
    wxPoint pos = event.GetPosition();
    wxSize size = GetSize();
    wxRect windowRect(0, 0, size.GetWidth(), size.GetHeight());
    
    if (windowRect.Contains(pos)) {
        // Click is inside the window, don't dismiss
        LOG_INF("Click inside floating window, not dismissing", "FlatUIFloatingWindow");
        return false; // Let the event be processed normally
    } else {
        // Click is outside, dismiss the window
        LOG_INF("Click outside floating window, dismissing", "FlatUIFloatingWindow");
        return wxPopupTransientWindow::ProcessLeftDown(event);
    }
}