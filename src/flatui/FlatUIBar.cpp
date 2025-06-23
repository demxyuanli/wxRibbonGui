#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUISpacerControl.h"
#include <string>
#include <numeric>
#include <wx/dcbuffer.h>
#include <wx/timer.h>
#include <wx/graphics.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <logger/Logger.h>
#include "config/ConstantsConfig.h"
#include "flatui/FlatUIPinControl.h"
#include <memory> // Required for std::unique_ptr and std::move
#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)
#define CFG_FONTNAME() ConstantsConfig::getInstance().getDefaultFontFaceName()
#define CFG_DEFAULTFONT() ConstantsConfig::getInstance().getDefaultFont()

int FlatUIBar::GetBarHeight()
{
    return CFG_INT("BarRenderHeight");
}

FlatUIBar::FlatUIBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxControl(parent, id, pos, size, style | wxBORDER_NONE),
    m_activePage(0),
    m_homeSpace(nullptr),
    m_functionSpace(nullptr),
    m_profileSpace(nullptr),
    m_systemButtons(nullptr),
    m_tabFunctionSpacer(nullptr),
    m_functionProfileSpacer(nullptr),
    m_pinControl(nullptr),
    m_tabStyle(TabStyle::DEFAULT),
    m_tabBorderStyle(TabBorderStyle::SOLID),
    m_tabBorderTop(0),
    m_tabBorderBottom(0),
    m_tabBorderLeft(0),
    m_tabBorderRight(0),
    m_tabCornerRadius(0),
    m_functionSpaceCenterAlign(false),
    m_profileSpaceRightAlign(false),
    m_temporarilyShownPage(nullptr),
    m_isGlobalPinned(true)  // Default to pinned state (all content visible)
{
    SetName("FlatUIBar");  // Set a meaningful name for the bar itself
    SetFont(CFG_DEFAULTFONT());
    auto& cfg = ConstantsConfig::getInstance();
    m_tabBorderColour = CFG_COLOUR("BarTabBorderColour");
    m_tabBorderTopColour = CFG_COLOUR("BarActiveTabTopBorderColour");
    m_tabBorderBottomColour = CFG_COLOUR("BarTabBorderColour");
    m_tabBorderLeftColour = CFG_COLOUR("BarTabBorderColour");
    m_tabBorderRightColour = CFG_COLOUR("BarTabBorderColour");
    m_activeTabBgColour = CFG_COLOUR("ActBarBackgroundColour");
    m_activeTabTextColour = CFG_COLOUR("BarActiveTextColour");
    m_inactiveTabTextColour = CFG_COLOUR("BarInactiveTextColour");
    m_barTopMargin = CFG_INT("BarTopMargin");
    m_barBottomMargin = CFG_INT("BarBottomMargin");
    m_tabTopSpacing = CFG_INT("TabTopSpacing");

#ifdef __WXMSW__
    HWND hwnd = (HWND)GetHandle();
    if (hwnd) {
        long exStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_COMPOSITED);
    }
#endif

    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    SetBarTopMargin(0);
    SetBarBottomMargin(1);

    // Create child component controls
    m_homeSpace = new FlatUIHomeSpace(this, wxID_ANY);
    m_functionSpace = new FlatUIFunctionSpace(this, wxID_ANY);
    m_profileSpace = new FlatUIProfileSpace(this, wxID_ANY);
    m_systemButtons = new FlatUISystemButtons(this, wxID_ANY);

    // Set names for all controls
    m_homeSpace->SetName("HomeSpace");
    m_functionSpace->SetName("FunctionSpace");
    m_profileSpace->SetName("ProfileSpace");
    m_systemButtons->SetName("SystemButtons");

    m_homeSpace->SetDoubleBuffered(true);
    m_functionSpace->SetDoubleBuffered(true);
    m_profileSpace->SetDoubleBuffered(true);
    m_systemButtons->SetDoubleBuffered(true);

    // Create and set up the global pin control
    m_pinControl = new FlatUIPinControl(this, wxID_ANY);
    m_pinControl->SetName("GlobalPinControl");
    m_pinControl->SetDoubleBuffered(true);
    m_pinControl->SetPinned(m_isGlobalPinned); // Set initial state
    m_pinControl->Show(true);

    m_tabFunctionSpacer = new FlatUISpacerControl(this, 10);
    m_functionProfileSpacer = new FlatUISpacerControl(this, 10);
    m_tabFunctionSpacer->SetName("TabFunctionSpacer");
    m_functionProfileSpacer->SetName("FunctionProfileSpacer");

    m_tabFunctionSpacer->SetDoubleBuffered(true);
    m_functionProfileSpacer->SetDoubleBuffered(true);

    m_tabFunctionSpacer->SetCanDragWindow(true);
    m_functionProfileSpacer->SetCanDragWindow(true);

    m_tabFunctionSpacer->Hide();
    m_functionProfileSpacer->Hide();

    // m_pages is default constructed (empty wxVector)

    FlatUIEventManager::getInstance().bindBarEvents(this);

    FlatUIEventManager::getInstance().bindHomeSpaceEvents(m_homeSpace);
    FlatUIEventManager::getInstance().bindSystemButtonsEvents(m_systemButtons);
    FlatUIEventManager::getInstance().bindFunctionSpaceEvents(m_functionSpace);
    FlatUIEventManager::getInstance().bindProfileSpaceEvents(m_profileSpace);

    int barHeight = GetBarHeight() + 2; // Add 2 for border
    SetMinSize(wxSize(-1, barHeight));

    // Ensure child controls are initially hidden if they don't have content
    // or based on some initial state. FlatUIBar will Show() them as needed during layout.
    m_functionSpace->Show(false);
    m_profileSpace->Show(false);

    // Bind the new global pin control event
    Bind(wxEVT_PIN_STATE_CHANGED, &FlatUIBar::OnPinControlStateChanged, this, m_pinControl->GetId());

    // Setup global mouse capture for outside clicks
    SetupGlobalMouseCapture();

    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }

    Bind(wxEVT_SHOW, &FlatUIBar::OnShow, this);
}

FlatUIBar::~FlatUIBar() {
    // Unbind the show event
    Unbind(wxEVT_SHOW, &FlatUIBar::OnShow, this);

    FlatUIEventManager::getInstance().unbindBarEvents(this);
    FlatUIEventManager::getInstance().unbindHomeSpaceEvents(m_homeSpace);
    FlatUIEventManager::getInstance().unbindSystemButtonsEvents(m_systemButtons);
    FlatUIEventManager::getInstance().unbindFunctionSpaceEvents(m_functionSpace);
    FlatUIEventManager::getInstance().unbindProfileSpaceEvents(m_profileSpace);

    // Clear all pages
    m_pages.clear();
}

void FlatUIBar::OnShow(wxShowEvent& event)
{
    if (event.IsShown() && m_activePage < m_pages.size() && m_pages[m_activePage]) {
        CallAfter([this]() {
            if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
                FlatUIPage* currentPage = m_pages[m_activePage];
                currentPage->SetActive(true);

                // Show logic considering pinned and temporarily shown states
                if (m_isGlobalPinned || m_temporarilyShownPage == currentPage) {
                    if (!currentPage->IsShown()) { // Only show if not already shown
                        currentPage->Show();
                    }
                }
                else {
                    // If not pinned and not the designated temporarily shown page,
                    // ensure it's hidden (it might have been shown by a previous SetActivePage call
                    // before a global click hid it, and then FlatUIBar::Show is called)
                    if (currentPage->IsShown()) {
                        currentPage->Hide();
                    }
                }

                wxSize barClientSize = GetClientSize();
                int barStripHeight = GetBarHeight();
                currentPage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));
                int pageHeight = barClientSize.GetHeight() - barStripHeight - m_barTopMargin;
                if (pageHeight < 0) pageHeight = 0;
                currentPage->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));
                currentPage->Layout();
                currentPage->Refresh();

                UpdateElementPositionsAndSizes(GetClientSize());
                Refresh();
            }
            });
    }
    event.Skip();
}

wxSize FlatUIBar::DoGetBestSize() const
{
    wxSize bestSize(0, 0);
    bestSize.SetHeight(GetBarHeight() + m_barTopMargin); // Include top margin

    // Add the height of the active page ONLY if it's shown and not hidden
    if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
        FlatUIPage* currentPage = m_pages[m_activePage];

        // Only include page height if the page is actually shown
        if (currentPage->IsShown()) {
            wxSize pageSize = currentPage->GetBestSize();
            bestSize.SetHeight(bestSize.GetHeight() + pageSize.GetHeight());
            bestSize.SetWidth(wxMax(bestSize.GetWidth(), pageSize.GetWidth()));
        }
    }

    // Width calculation fallback
    if (bestSize.GetWidth() <= 0) {
        bestSize.SetWidth(wxWindow::DoGetBestSize().GetWidth());
        if (bestSize.GetWidth() <= 0) {
            bestSize.SetWidth(200); // Minimum reasonable width
        }
    }

    // Ensure minimum height is at least the bar height plus margin
    if (bestSize.GetHeight() < (GetBarHeight() + m_barTopMargin)) {
        bestSize.SetHeight(GetBarHeight() + m_barTopMargin);
    }

    return bestSize;
}

void FlatUIBar::AddPage(FlatUIPage* page)
{
    if (!page) return;

    m_pages.push_back(page);

    page->Hide();

    if (m_pages.size() == 1) {
        m_activePage = 0;
        page->SetActive(true);
    }
    else {
        page->SetActive(false);
    }

    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::SetActivePage(size_t pageIndex)
{
    if (pageIndex >= m_pages.size() || !m_pages[pageIndex]) {
        return;
    }

    // Hide current active page if different
    if (m_activePage < m_pages.size() && m_pages[m_activePage] && m_activePage != pageIndex) {
        FlatUIPage* oldPage = m_pages[m_activePage];
        oldPage->SetActive(false);
        oldPage->Hide();
    }

    m_activePage = pageIndex;
    FlatUIPage* newPage = m_pages[m_activePage];

    // Configure the new page
    wxSize barClientSize = GetClientSize();
    int barStripHeight = GetBarHeight();
    newPage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));
    int pageHeight = barClientSize.GetHeight() - barStripHeight - m_barTopMargin;
    if (pageHeight < 0) pageHeight = 0;
    newPage->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));

    newPage->SetActive(true);

    // Show page based on global pin state
    if (m_isGlobalPinned) {
        // In pinned state, always show the active page
        newPage->Show();
        m_temporarilyShownPage = nullptr;
        LOG_INF("Pinned state: Showing active page - " + newPage->GetLabel().ToStdString(), "FlatUIBar");
    }
    else {
        // In unpinned state, a click on a tab should temporarily show the page
        if (m_temporarilyShownPage && m_temporarilyShownPage != newPage) {
            m_temporarilyShownPage->Hide(); // Hide the old temporarily shown page
            LOG_INF("Unpinned state: Hiding previous temporarily shown page", "FlatUIBar");
        }
        newPage->Show();
        m_temporarilyShownPage = newPage;
        LOG_INF("Unpinned state: Temporarily showing page - " + newPage->GetLabel().ToStdString(), "FlatUIBar");
    }

    newPage->Layout();
    newPage->UpdateLayout();

    LOG_INF("Activated page at index: " + std::to_string(pageIndex) +
        ", global_pinned: " + std::string(m_isGlobalPinned ? "true" : "false"), "FlatUIBar");

    InvalidateBestSize();
    wxWindow* parent = GetParent();
    if (parent) {
        parent->InvalidateBestSize();
        parent->Layout();
        parent->Refresh();
    }
    Layout();
    Refresh();
}

size_t FlatUIBar::GetPageCount() const noexcept { return m_pages.size(); }
FlatUIPage* FlatUIBar::GetPage(size_t index) const { return (index < m_pages.size()) ? m_pages[index] : nullptr; }

void FlatUIBar::OnSize(wxSizeEvent& evt)
{
    wxSize newSize = GetClientSize();
    UpdateElementPositionsAndSizes(newSize);

    if (m_homeSpace) m_homeSpace->Update();
    if (m_functionSpace) m_functionSpace->Update();
    if (m_profileSpace) m_profileSpace->Update();
    if (m_systemButtons) m_systemButtons->Update();
    if (m_tabFunctionSpacer) m_tabFunctionSpacer->Update();
    if (m_functionProfileSpacer) m_functionProfileSpacer->Update();

    if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
        m_pages[m_activePage]->Update();
    }

    Refresh(true);
    Update();

    evt.Skip();
}

void FlatUIBar::OnPinControlStateChanged(wxCommandEvent& event)
{
    ToggleGlobalPinState();
    event.Skip();
}

bool FlatUIBar::IsBarPinned() const
{
    return m_isGlobalPinned;
}

void FlatUIBar::OnGlobalMouseDown(wxMouseEvent& event)
{
    if (!this || !IsShown()) {
        event.Skip();
        return;
    }

    // Only handle global clicks when in unpinned state
    if (m_isGlobalPinned) {
        event.Skip();
        return;
    }

    wxPoint clickPos = event.GetPosition(); // Screen coordinates
    wxWindow* clickedWindow = wxFindWindowAtPoint(clickPos);

    // Check if the click was inside the FlatUIBar or its pages
    bool clickInsideBarArea = false;
    if (clickedWindow) {
        wxWindow* current = clickedWindow;
        while (current) {
            if (current == this) {
                clickInsideBarArea = true;
                break;
            }
            // Check if click was on pin control - should not hide pages
            if (m_pinControl && current == m_pinControl) {
                clickInsideBarArea = true;
                break;
            }
            // Check if click was on any of the pages
            for (auto& page : m_pages) {
                if (page && current == page) {
                    clickInsideBarArea = true;
                    break;
                }
            }
            if (clickInsideBarArea) break;
            current = current->GetParent();
        }
    }

    if (!clickInsideBarArea) {
        // Click outside bar area in unpinned state - hide all content
        LOG_INF("Global click outside bar area in unpinned state - hiding content", "FlatUIBar");
        HideAllContentExceptBarSpace();
        InvalidateBestSize();
        wxWindow* parent = GetParent();
        if (parent) {
            parent->InvalidateBestSize();
            parent->Layout();
            parent->Refresh();
        }
        Layout();
        Refresh();
    }

    event.Skip();
}


bool FlatUIBar::IsPointInBarArea(const wxPoint& globalPoint) const
{
    wxPoint localPoint = ScreenToClient(globalPoint);
    wxRect barRect(0, 0, GetSize().GetWidth(), GetBarHeight());
    return barRect.Contains(localPoint);
}

void FlatUIBar::SetupGlobalMouseCapture()
{
    wxWindow* topLevel = wxGetTopLevelParent(this);
    if (topLevel) {
        topLevel->Bind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
    }
}

void FlatUIBar::ReleaseGlobalMouseCapture()
{
    wxWindow* topLevel = wxGetTopLevelParent(this);
    if (topLevel) {
        topLevel->Unbind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
    }
}

void FlatUIBar::SetGlobalPinned(bool pinned)
{
    if (m_isGlobalPinned != pinned) {
        m_isGlobalPinned = pinned;
        
        // Update pin control visual state
        if (m_pinControl) {
            m_pinControl->SetPinned(pinned);
        }
        
        // Apply pin state logic
        OnGlobalPinStateChanged(pinned);

        LOG_INF("Global pin state changed to: " + std::string(pinned ? "pinned" : "unpinned"), "FlatUIBar");
    }
}

void FlatUIBar::ToggleGlobalPinState()
{
    SetGlobalPinned(!m_isGlobalPinned);
}

bool FlatUIBar::ShouldShowPages() const
{
    // Pages should be visible if:
    // 1. Globally pinned (always show)
    // 2. Not pinned but there's a temporarily shown page
    return m_isGlobalPinned || (m_temporarilyShownPage != nullptr);
}

void FlatUIBar::OnGlobalPinStateChanged(bool isPinned)
{
    LOG_INF("OnGlobalPinStateChanged called with isPinned: " + std::string(isPinned ? "true" : "false"), "FlatUIBar");

    if (isPinned) {
        // Pinned state: Show all content including active page
        ShowAllContent();
        LOG_INF("Switched to pinned state - showing all content", "FlatUIBar");
    }
    else {
        // Unpinned state: Hide all content except bar space (tabs area)
        HideAllContentExceptBarSpace();
        LOG_INF("Switched to unpinned state - hiding all content except bar space", "FlatUIBar");
    }

    // Update layout and refresh
    InvalidateBestSize();
    wxWindow* parent = GetParent();
    if (parent) {
        parent->InvalidateBestSize();
        parent->Layout();
        parent->Refresh();
    }
    Layout();
    Refresh();
}

void FlatUIBar::ShowAllContent()
{
    LOG_INF("ShowAllContent: Showing all page content", "FlatUIBar");
    
    // Show the active page if there is one
    if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
        FlatUIPage* activePage = m_pages[m_activePage];
        
        LOG_INF("ShowAllContent: Active page found - " + activePage->GetLabel().ToStdString(), "FlatUIBar");
        
        // Always show the active page in pinned mode
        if (!activePage->IsShown()) {
            activePage->Show();
            LOG_INF("ShowAllContent: Showed active page", "FlatUIBar");
        }

        // Position and size the active page
        wxSize barClientSize = GetClientSize();
        int barStripHeight = GetBarHeight();
        activePage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));
        int pageHeight = barClientSize.GetHeight() - barStripHeight - m_barTopMargin;
        if (pageHeight < 0) pageHeight = 0;
        activePage->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));
        activePage->Layout();
        activePage->UpdateLayout();
        
        LOG_INF("ShowAllContent: Positioned and sized active page", "FlatUIBar");
    }

    // Clear temporarily shown page state since we're in pinned mode
    m_temporarilyShownPage = nullptr;
    LOG_INF("ShowAllContent: Cleared temporarily shown page state", "FlatUIBar");
}

void FlatUIBar::HideAllContentExceptBarSpace()
{
    LOG_INF("HideAllContentExceptBarSpace: Hiding all page content", "FlatUIBar");
    
    // Hide all pages
    for (auto& page : m_pages) {
        if (page && page->IsShown()) {
            LOG_INF("HideAllContentExceptBarSpace: Hiding page - " + page->GetLabel().ToStdString(), "FlatUIBar");
            page->Hide();
        }
    }

    // Clear temporarily shown page state
    m_temporarilyShownPage = nullptr;
    LOG_INF("HideAllContentExceptBarSpace: Cleared temporarily shown page state", "FlatUIBar");
}