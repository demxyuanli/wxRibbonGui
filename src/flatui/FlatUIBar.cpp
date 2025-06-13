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
    : wxControl(parent, id, pos, size, style | wxBORDER_DEFAULT),
    m_activePage(0),
    m_homeSpace(nullptr),
    m_functionSpace(nullptr),
    m_profileSpace(nullptr),
    m_systemButtons(nullptr),
    m_tabFunctionSpacer(nullptr),
    m_functionProfileSpacer(nullptr),
    m_tabStyle(TabStyle::DEFAULT),
    m_tabBorderStyle(TabBorderStyle::SOLID),
    m_tabBorderTop(0),
    m_tabBorderBottom(0),
    m_tabBorderLeft(0),
    m_tabBorderRight(0),
    m_tabCornerRadius(0),
    m_functionSpaceCenterAlign(false),
    m_profileSpaceRightAlign(false),
    m_temporarilyShownPage(nullptr)
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

    Bind(wxEVT_COMMAND_MENU_SELECTED, &FlatUIBar::OnPagePinStateChanged, this);

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
                if (currentPage->IsPinned() || m_temporarilyShownPage == currentPage) {
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

    // Check if any pinned pages are visible and should contribute to size
    for (const auto& page : m_pages) {
        if (page && page->IsShown() && page->IsPinned()) {
            wxSize pageSize = page->GetBestSize();
            // For pinned pages, we might want to consider their height
            // This depends on your UI design - whether pinned pages stack or overlay
            bestSize.SetHeight(wxMax(bestSize.GetHeight(), GetBarHeight() + m_barTopMargin + pageSize.GetHeight()));
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

    // Hide any previously temporarily shown page if it's not the new active page or if the new page is pinned
    if (m_temporarilyShownPage && m_temporarilyShownPage != m_pages[pageIndex]) {
        if (!m_temporarilyShownPage->IsPinned()) { // Ensure it wasn't pinned in the meantime
            HideTemporarilyShownPage();
        }
    }

    // Hide current active page if it's different, not pinned, and not the new page
    if (m_activePage < m_pages.size() && m_pages[m_activePage] && m_activePage != pageIndex) {
        FlatUIPage* oldPage = m_pages[m_activePage];
        if (oldPage != m_temporarilyShownPage) { // Don't hide if it was the temp page, already handled
            oldPage->SetActive(false);
            if (!oldPage->IsPinned()) {
                oldPage->Hide();
            }
        }
    }

    m_activePage = pageIndex;
    FlatUIPage* newPage = m_pages[m_activePage];

    // Configure and show the new page
    wxSize barClientSize = GetClientSize();
    int barStripHeight = GetBarHeight();
    newPage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));
    int pageHeight = barClientSize.GetHeight() - barStripHeight - m_barTopMargin;
    if (pageHeight < 0) pageHeight = 0;
    newPage->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));

    newPage->SetActive(true);
    newPage->Show(); // Always show the content of the active tab initially

    if (!newPage->IsPinned()) {
        m_temporarilyShownPage = newPage; // Mark as temporarily shown if not pinned
    }
    else {
        m_temporarilyShownPage = nullptr; // If it's pinned, it's not temporary
    }

    newPage->Layout();
    newPage->UpdateLayout(); // For pin control positioning

    LOG_INF("Activated page at index: " + std::to_string(pageIndex) + ", pinned: " +
        std::string(newPage->IsPinned() ? "true" : "false") +
        ", temp_shown: " + std::string(m_temporarilyShownPage == newPage ? "true" : "false"), "FlatUIBar");

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

void FlatUIBar::HideTemporarilyShownPage() {
    if (m_temporarilyShownPage && !m_temporarilyShownPage->IsPinned()) {
        LOG_INF("Hiding temporarily shown page: " + m_temporarilyShownPage->GetLabel().ToStdString(), "FlatUIBar");
        m_temporarilyShownPage->Hide();
        // Important: If hiding the temp page makes it inactive visually (e.g. tab appearance changes),
        // you might need to call m_temporarilyShownPage->SetActive(false) here too, depending on desired UI.
        // However, the tab itself (m_activePage) remains the active one until another is clicked.
    }
    m_temporarilyShownPage = nullptr;
    // After hiding a page, the bar's best size might change.
    InvalidateBestSize();
    wxWindow* parent = GetParent();
    if (parent) {
        parent->InvalidateBestSize();
        parent->Layout();
        // parent->Refresh(); // Refresh might be redundant if Layout() handles it
    }
    Layout(); // Our own layout might need update
    Refresh(); // Refresh the bar to reflect changes
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

void FlatUIBar::OnPagePinStateChanged(wxCommandEvent& event)
{
    if (event.GetString() == "PAGE_PIN_STATE_CHANGED") {
        FlatUIPage* changedPage = dynamic_cast<FlatUIPage*>(event.GetEventObject());
        if (!changedPage) {
            LOG_ERR("Invalid page object in pin state change event", "FlatUIBar");
            return;
        }

        bool isPinned = event.GetInt() == 1;
        LOG_INF("Page " + changedPage->GetLabel().ToStdString() + " pin state changed to: " + std::string(isPinned ? "pinned" : "unpinned"), "FlatUIBar");

        if (isPinned) {
            // If page was pinned, ensure it's visible and no longer temporary
            if (m_temporarilyShownPage == changedPage) {
                m_temporarilyShownPage = nullptr; // No longer temporarily shown
            }
            if (!changedPage->IsShown()) {
                changedPage->Show(); // Ensure it's shown
            }
            // Reposition/resize if necessary (already handled by SetActivePage if it's active)
            // If it's not the active page but gets pinned, it should just become visible if it wasn't.
            // The current SetActivePage logic might need review if a non-active page can be pinned
            // and should immediately affect layout beyond just showing its tab.
        }
        else {
            // If page was unpinned
            if (changedPage == m_pages[m_activePage]) {
                // If the active page is unpinned, it becomes temporarily shown
                m_temporarilyShownPage = changedPage;
            }
            else {
                // If a non-active page is unpinned, it should be hidden
                changedPage->Hide();
            }
        }

        // Adjust layout for the changed page
        wxSize barClientSize = GetClientSize();
        int barStripHeight = GetBarHeight();
        changedPage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));
        int pageHeight = barClientSize.GetHeight() - barStripHeight - m_barTopMargin;
        if (pageHeight < 0) pageHeight = 0;
        changedPage->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));
        changedPage->Layout();
        changedPage->UpdateLayout();

        InvalidateBestSize();
        wxWindow* parent = GetParent();
        if (parent) {
            parent->InvalidateBestSize();
            parent->Layout();
            parent->Refresh();
        }
        UpdateElementPositionsAndSizes(GetClientSize());
        Layout();
        Refresh();
    }
}

bool FlatUIBar::AnyPagePinned() const
{
    for (const auto& page : m_pages) {
        if (page && page->IsPinned()) {
            return true;
        }
    }
    return false;
}

bool FlatUIBar::IsBarPinned() const
{
    return AnyPagePinned();
}

void FlatUIBar::OnGlobalMouseDown(wxMouseEvent& event)
{
    if (!this || !IsShown()) {
        event.Skip();
        return;
    }
    if (!m_temporarilyShownPage) {
        event.Skip(); // Nothing to do if no page is temporarily shown
        return;
    }

    wxPoint clickPos = event.GetPosition(); // Screen coordinates
    wxWindow* clickedWindow = wxFindWindowAtPoint(clickPos);

    // Check if the click was inside the FlatUIBar itself or its temporarily shown page
    bool clickInsideBarOrTempPage = false;
    if (clickedWindow) {
        wxWindow* current = clickedWindow;
        while (current) {
            if (current == this || current == m_temporarilyShownPage) {
                clickInsideBarOrTempPage = true;
                break;
            }
            current = current->GetParent();
        }
    }

    if (!clickInsideBarOrTempPage) {
        HideTemporarilyShownPage();
    }
    // We don't skip the event here, as other windows might need to process it.
    // The original event.Skip() was there when the logic was simpler.
    // Depending on exact needs, you might re-evaluate if Skip() is needed.
    event.Skip();
}

// ... existing code ...
void FlatUIBar::HideAllPages() // This function might be redundant or need re-evaluation with temp pages
{
    bool anyPageHidden = false;
    if (m_temporarilyShownPage) {
        HideTemporarilyShownPage(); // Prioritize hiding the temp page
        anyPageHidden = true; // Assume it was hidden
    }
    if (anyPageHidden) {
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
}
// ... existing code ...

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
