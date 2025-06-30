#include "flatui/FlatUIFixPanel.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIUnpinButton.h"
#include "logger/Logger.h"
#include "config/ConstantsConfig.h"
#include <wx/dcbuffer.h>

#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)

wxBEGIN_EVENT_TABLE(FlatUIFixPanel, wxPanel)
    EVT_SIZE(FlatUIFixPanel::OnSize)
    EVT_PAINT(FlatUIFixPanel::OnPaint)
wxEND_EVENT_TABLE()

FlatUIFixPanel::FlatUIFixPanel(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxBORDER_NONE),
    m_activePageIndex(wxNOT_FOUND),
    m_unpinButton(nullptr)
{
    SetName("FlatUIFixPanel");
    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(CFG_COLOUR("ActBarBackgroundColour"));

#ifdef __WXMSW__
    HWND hwnd = (HWND)GetHandle();
    if (hwnd) {
        long exStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_COMPOSITED);
    }
#endif

    // Create unpin button
    m_unpinButton = new FlatUIUnpinButton(this, wxID_ANY);
    m_unpinButton->SetName("FixPanelUnpinButton");
    m_unpinButton->SetDoubleBuffered(true);
    m_unpinButton->Show(true); // Initially shown when fix panel is visible
    
    // Ensure unpin button is on top by default
    m_unpinButton->Raise();

    LOG_INF("Created FlatUIFixPanel with unpin button", "FlatUIFixPanel");
}

FlatUIFixPanel::~FlatUIFixPanel()
{
    // Pages are owned by their creator, don't delete them here
    // Just clear the vector
    m_pages.clear();
    
    if (m_unpinButton) {
        m_unpinButton->Destroy();
        m_unpinButton = nullptr;
    }
    
    LOG_INF("Destroyed FlatUIFixPanel", "FlatUIFixPanel");
}

void FlatUIFixPanel::AddPage(FlatUIPage* page)
{
    if (!page) {
        return;
    }

    // Check if page already exists to avoid duplicates
    for (auto* existingPage : m_pages) {
        if (existingPage == page) {
            LOG_DBG("Page '" + page->GetLabel().ToStdString() + "' already exists in FixPanel, skipping", "FlatUIFixPanel");
            return;
        }
    }

    // Reparent the page to this fix panel
    page->Reparent(this);
    m_pages.push_back(page);

    // Hide the page initially
    page->Hide();

    // If this is the first page, make it active
    if (m_pages.size() == 1) {
        SetActivePage(static_cast<size_t>(0));
    }

    LOG_INF("Added page '" + page->GetLabel().ToStdString() + "' to FixPanel", "FlatUIFixPanel");
    RecalculateSize();
}

void FlatUIFixPanel::SetActivePage(size_t pageIndex)
{
    if (pageIndex >= m_pages.size()) {
        return;
    }

    // Quick exit if already active
    if (m_activePageIndex == pageIndex && m_activePageIndex < m_pages.size() && 
        m_pages[m_activePageIndex] && m_pages[m_activePageIndex]->IsShown()) {
        return;
    }

    // Batch all page changes to minimize visual updates
    Freeze();
    
    try {
        // Hide current active page
        if (m_activePageIndex < m_pages.size() && m_pages[m_activePageIndex]) {
            m_pages[m_activePageIndex]->SetActive(false);
            m_pages[m_activePageIndex]->Hide();
        }

        // Show new active page
        m_activePageIndex = pageIndex;
        FlatUIPage* newActivePage = m_pages[m_activePageIndex];
        if (newActivePage) {
            newActivePage->SetActive(true);
            newActivePage->Show();
            PositionActivePage();
            
            LOG_INF("Set active page to '" + newActivePage->GetLabel().ToStdString() + "'", "FlatUIFixPanel");
        }
    }
    catch (...) {
        Thaw();
        throw;
    }
    
    Thaw();
    
    // Defer layout update to avoid multiple calls
    CallAfter([this]() {
        UpdateLayout();
    });
}

void FlatUIFixPanel::SetActivePage(FlatUIPage* page)
{
    if (!page) {
        return;
    }

    for (size_t i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i] == page) {
            SetActivePage(static_cast<size_t>(i));
            return;
        }
    }
}

FlatUIPage* FlatUIFixPanel::GetActivePage() const
{
    if (m_activePageIndex < m_pages.size()) {
        return m_pages[m_activePageIndex];
    }
    return nullptr;
}

FlatUIPage* FlatUIFixPanel::GetPage(size_t index) const
{
    if (index < m_pages.size()) {
        return m_pages[index];
    }
    return nullptr;
}

void FlatUIFixPanel::UpdateLayout()
{
    if (!IsShown()) {
        return;
    }

    // Only update if there are actual changes needed
    FlatUIPage* activePage = GetActivePage();
    if (!activePage) {
        return;
    }

    Freeze();
    
    PositionActivePage();
    PositionUnpinButton();
    
    Thaw();
    
    // Use deferred refresh to batch multiple layout updates
    CallAfter([this]() {
        if (IsShown()) {
            Refresh(false);
        }
    });
}

void FlatUIFixPanel::RecalculateSize()
{
    if (m_pages.empty()) {
        SetMinSize(wxSize(100, 50));
        return;
    }

    // Calculate the maximum size needed by any page
    wxSize maxSize(0, 0);
    for (auto* page : m_pages) {
        if (page) {
            wxSize pageSize = page->GetBestSize();
            maxSize.SetWidth(wxMax(maxSize.GetWidth(), pageSize.GetWidth()));
            maxSize.SetHeight(wxMax(maxSize.GetHeight(), pageSize.GetHeight()));
        }
    }

    SetMinSize(maxSize);
    LOG_INF("RecalculateSize: FixPanel size set to (" + 
           std::to_string(maxSize.GetWidth()) + "," + 
           std::to_string(maxSize.GetHeight()) + ")", "FlatUIFixPanel");
}

void FlatUIFixPanel::ShowUnpinButton(bool show)
{
    if (m_unpinButton) {
        m_unpinButton->Show(show);
        if (show) {
            PositionUnpinButton();
            m_unpinButton->Raise(); // Ensure it's on top when shown
        }
        LOG_INF("Unpin button " + std::string(show ? "shown" : "hidden"), "FlatUIFixPanel");
    }
}

wxSize FlatUIFixPanel::DoGetBestSize() const
{
    if (m_pages.empty()) {
        return wxSize(100, 50);
    }

    // Return the size of the largest page
    wxSize maxSize(0, 0);
    for (const auto* page : m_pages) {
        if (page) {
            wxSize pageSize = page->GetBestSize();
            maxSize.SetWidth(wxMax(maxSize.GetWidth(), pageSize.GetWidth()));
            maxSize.SetHeight(wxMax(maxSize.GetHeight(), pageSize.GetHeight()));
        }
    }

    return maxSize;
}

void FlatUIFixPanel::OnSize(wxSizeEvent& event)
{
    UpdateLayout();
    event.Skip();
}

void FlatUIFixPanel::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetSize();

    // Fill background
    dc.SetBackground(wxBrush(GetBackgroundColour()));
    dc.Clear();

    // Draw border if needed
    // dc.SetPen(wxPen(CFG_COLOUR("PanelBorderColour"), CFG_INT("PanelBorderWidth")));
    // dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    event.Skip();
}

void FlatUIFixPanel::PositionUnpinButton()
{
    if (!m_unpinButton || !m_unpinButton->IsShown()) {
        return;
    }

    wxSize panelSize = GetSize();
    wxSize buttonSize = m_unpinButton->GetBestSize();
    
    // Position at bottom-right corner with small margin
    int margin = 2;
    wxPoint buttonPos(
        panelSize.GetWidth() - buttonSize.GetWidth() - margin,
        panelSize.GetHeight() - buttonSize.GetHeight() - margin
    );
    
    m_unpinButton->SetPosition(buttonPos);
    m_unpinButton->SetSize(buttonSize);
    
    // Ensure button is on top of other controls
    m_unpinButton->Raise();
    
    LOG_DBG("Positioned unpin button at (" + 
           std::to_string(buttonPos.x) + "," + 
           std::to_string(buttonPos.y) + ") and raised to top", "FlatUIFixPanel");
}

void FlatUIFixPanel::PositionActivePage()
{
    FlatUIPage* activePage = GetActivePage();
    if (!activePage) {
        return;
    }

    wxSize panelSize = GetSize();
    wxSize buttonSize(0, 0);
    int margin = 5;
    
    // Reserve space for unpin button if it's shown
    if (m_unpinButton && m_unpinButton->IsShown()) {
        buttonSize = m_unpinButton->GetBestSize();
    }
    
    // Page takes up the entire panel area except for button area
    activePage->SetPosition(wxPoint(0, 0));
    activePage->SetSize(panelSize);
    activePage->Layout();
    
    // Ensure unpin button is on top by raising it after positioning the page
    if (m_unpinButton && m_unpinButton->IsShown()) {
        m_unpinButton->Raise();
    }
    
    LOG_DBG("Positioned active page '" + activePage->GetLabel().ToStdString() + 
           "' at size (" + std::to_string(panelSize.GetWidth()) + "," + 
           std::to_string(panelSize.GetHeight()) + ") and raised unpin button", "FlatUIFixPanel");
}

void FlatUIFixPanel::HideAllPages()
{
    for (auto* page : m_pages) {
        if (page && page->IsShown()) {
            page->Hide();
        }
    }
}

void FlatUIFixPanel::ClearContent()
{
    LOG_INF("Clearing FixPanel content", "FlatUIFixPanel");
    
    Freeze();
    
    // Hide and deactivate all pages
    for (auto* page : m_pages) {
        if (page) {
            page->SetActive(false);
            page->Hide();
            // Don't destroy the page, just reparent it back to its original parent if needed
        }
    }
    
    // Clear the pages vector (pages are owned by their original creators)
    m_pages.clear();
    
    // Reset active page index
    m_activePageIndex = wxNOT_FOUND;
    
    // Hide unpin button
    if (m_unpinButton) {
        m_unpinButton->Hide();
    }
    
    Thaw();
    
    LOG_INF("FixPanel content cleared", "FlatUIFixPanel");
}

void FlatUIFixPanel::ResetState()
{
    LOG_INF("Resetting FixPanel state", "FlatUIFixPanel");
    
    Freeze();
    
    // Hide all pages and deactivate them
    for (auto* page : m_pages) {
        if (page) {
            page->SetActive(false);
            page->Hide();
        }
    }
    
    // Reset active page index but keep pages
    m_activePageIndex = wxNOT_FOUND;
    
    // Hide unpin button
    if (m_unpinButton) {
        m_unpinButton->Hide();
    }
    
    Thaw();
    
    LOG_INF("FixPanel state reset", "FlatUIFixPanel");
} 