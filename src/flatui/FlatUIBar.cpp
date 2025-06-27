#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUIEventManager.h"
#include "flatui/FlatUISpacerControl.h"
#include "flatui/FlatUIFloatPanel.h"
#include <string>
#include <numeric>
#include <wx/dcbuffer.h>
#include <wx/timer.h>
#include <wx/graphics.h>
#include <wx/dcmemory.h>
#include <wx/dcclient.h>
#include <logger/Logger.h>
#include "config/ConstantsConfig.h"
#include "flatui/FlatUIUnpinButton.h"
// FlatUIPinButton is now handled by FlatUIFloatPanel
#include <memory> // Required for std::unique_ptr and std::move
// Define the backward compatibility event
wxDEFINE_EVENT(wxEVT_PIN_STATE_CHANGED, wxCommandEvent);

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
    m_unpinButton(nullptr),
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
    m_isGlobalPinned(true),  // Default to pinned state (all content visible)
    m_barUnpinnedHeight(CFG_INT("BarUnpinnedHeight")),
    m_lastActivePageBeforeUnpin(0), // Initialize with a safe default
    m_activeFloatingPage(wxNOT_FOUND), // No floating page initially
    m_floatPanel(nullptr)
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

    // Create and set up the unpin button (pin button is now handled by float panel)
    m_unpinButton = new FlatUIUnpinButton(this, wxID_ANY);
    m_unpinButton->SetName("UnpinButton");
    m_unpinButton->SetDoubleBuffered(true);
    m_unpinButton->Show(m_isGlobalPinned); // Show when pinned

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

    // Create float panel
    m_floatPanel = new FlatUIFloatPanel(this);

    // Bind the unpin button event (pin button events come from float panel)
    Bind(wxEVT_UNPIN_BUTTON_CLICKED, &FlatUIBar::OnUnpinButtonClicked, this, m_unpinButton->GetId());
    
    // Bind pin button events from float panel
    Bind(wxEVT_PIN_BUTTON_CLICKED, &FlatUIBar::OnPinButtonClicked, this);
    
    // Bind float panel dismiss event
    Bind(wxEVT_FLOAT_PANEL_DISMISSED, &FlatUIBar::OnFloatPanelDismissed, this);

    // Setup global mouse capture
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

    // Unbind float panel dismiss event
    Unbind(wxEVT_FLOAT_PANEL_DISMISSED, &FlatUIBar::OnFloatPanelDismissed, this);

    // Release global mouse capture
    ReleaseGlobalMouseCapture();

    FlatUIEventManager::getInstance().unbindBarEvents(this);
    FlatUIEventManager::getInstance().unbindHomeSpaceEvents(m_homeSpace);
    FlatUIEventManager::getInstance().unbindSystemButtonsEvents(m_systemButtons);
    FlatUIEventManager::getInstance().unbindFunctionSpaceEvents(m_functionSpace);
    FlatUIEventManager::getInstance().unbindProfileSpaceEvents(m_profileSpace);
    
    if (m_floatPanel) {
        m_floatPanel->Destroy();
        m_floatPanel = nullptr;
    }

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

    // Determine if we need to show a page, which dictates the total height.
    if (ShouldShowPages()) {
        // Pinned, or unpinned with a temporary page: calculate full size.
        bestSize.SetHeight(GetBarHeight() + m_barTopMargin); 

        if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
            // Add the height of the active page.
            wxSize pageSize = m_pages[m_activePage]->GetBestSize();
            bestSize.SetHeight(bestSize.GetHeight() + pageSize.GetHeight());
            bestSize.SetWidth(wxMax(bestSize.GetWidth(), pageSize.GetWidth()));
        }
    } else {
        // Unpinned and no temporary page: use the collapsed height.
        bestSize.SetHeight(m_barUnpinnedHeight);
    }
    
    // Ensure the width is at least the parent's width.
    if (GetParent()) {
        bestSize.SetWidth(wxMax(bestSize.GetWidth(), GetParent()->GetClientSize().GetWidth()));
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

    if (m_activePage == pageIndex && m_pages[m_activePage]->IsShown()) {
        return; // Already active and shown, do nothing.
    }

    // Hide the old page if it's different
    if (m_activePage < m_pages.size() && m_pages[m_activePage] && m_activePage != pageIndex) {
        m_pages[m_activePage]->SetActive(false);
        m_pages[m_activePage]->Hide();
    }

    m_activePage = pageIndex;
    FlatUIPage* newPage = m_pages[m_activePage];
    newPage->SetActive(true);

    // This method should only be used for permanent page changes,
    // which implies a pinned state or a structural change.
    // The temporary display in unpinned state is handled by HandleTabAreaClick.
    if (m_isGlobalPinned) {
        if (!newPage->IsShown()) {
        newPage->Show();
        }
        m_temporarilyShownPage = nullptr; // Ensure no temporary page is set
    }
    
    // Trigger a full layout update.
    InvalidateBestSize();
    wxWindow* parent = GetParent();
    if (parent) {
        parent->Layout();
    }
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

void FlatUIBar::OnPinButtonClicked(wxCommandEvent& event)
{
    SetGlobalPinned(true);
    
    // Send the old compatible event for FlatFrame
    wxCommandEvent pinEvent(wxEVT_PIN_STATE_CHANGED, GetId());
    pinEvent.SetEventObject(this);
    pinEvent.SetInt(1); // pinned = true
    GetParent()->GetEventHandler()->ProcessEvent(pinEvent);
    
    event.Skip();
}

void FlatUIBar::OnUnpinButtonClicked(wxCommandEvent& event)
{
    SetGlobalPinned(false);
    
    // Send the old compatible event for FlatFrame
    wxCommandEvent pinEvent(wxEVT_PIN_STATE_CHANGED, GetId());
    pinEvent.SetEventObject(this);
    pinEvent.SetInt(0); // pinned = false
    GetParent()->GetEventHandler()->ProcessEvent(pinEvent);
    
    event.Skip();
}

bool FlatUIBar::IsBarPinned() const
{
    return m_isGlobalPinned;
}

void FlatUIBar::OnGlobalMouseDown(wxMouseEvent& event)
{
    // With wxPopupTransientWindow, we only need to handle clicks within the bar area
    // The floating window will automatically dismiss itself on outside clicks
    
    if (!this || !IsShown()) {
        event.Skip();
        return;
    }

    // Only handle clicks when in unpinned state
    if (m_isGlobalPinned) {
        event.Skip();
        return;
    }

    // Check if this is a click on the bar itself (for tab area empty space handling)
    wxPoint clickPos = event.GetPosition();
    wxWindow* clickedWindow = wxFindWindowAtPoint(clickPos);
    
    if (clickedWindow == this) {
        // This is handled by HandleTabAreaClick, so we don't need to do anything special here
        LOG_INF("Click on FlatUIBar detected", "FlatUIBar");
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
    // Bind to multiple possible parent windows to ensure we catch global clicks
    wxWindow* topLevel = wxGetTopLevelParent(this);
    if (topLevel) {
        topLevel->Bind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
        LOG_INF("Bound global mouse capture to top-level window: " + topLevel->GetName().ToStdString(), "FlatUIBar");
    }
    
    // Also bind to immediate parent if different from top-level
    wxWindow* parent = GetParent();
    if (parent && parent != topLevel) {
        parent->Bind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
        LOG_INF("Bound global mouse capture to parent window: " + parent->GetName().ToStdString(), "FlatUIBar");
    }
    
    // Bind to self as well for additional coverage
    this->Bind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
    LOG_INF("Bound global mouse capture to self", "FlatUIBar");
}

void FlatUIBar::ReleaseGlobalMouseCapture()
{
    wxWindow* topLevel = wxGetTopLevelParent(this);
    if (topLevel) {
        topLevel->Unbind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
    }
    
    // Also release immediate parent if different from top-level
    wxWindow* parent = GetParent();
    if (parent && parent != topLevel) {
        parent->Unbind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
    }
    
    // Unbind self
    this->Unbind(wxEVT_LEFT_DOWN, &FlatUIBar::OnGlobalMouseDown, this);
}

void FlatUIBar::SetGlobalPinned(bool pinned)
{
    if (m_isGlobalPinned != pinned) {
        
        if (!pinned) { // Transitioning to UNPINNED
            m_lastActivePageBeforeUnpin = m_activePage;
            m_activePage = wxNOT_FOUND; 
            m_activeFloatingPage = wxNOT_FOUND; // Reset floating page selection
        } else { // Transitioning to PINNED
            // Ensure float panel is hidden when pinning
            HideFloatPanel();
            m_activeFloatingPage = wxNOT_FOUND; // Reset floating page selection
            if (m_lastActivePageBeforeUnpin < m_pages.size()) { // Ensure index is valid
                m_activePage = m_lastActivePageBeforeUnpin;
            } else if (!m_pages.empty()) {
                m_activePage = 0; // Fallback to first page
            } else {
                m_activePage = wxNOT_FOUND; // No pages available
            }
        }
        
        m_isGlobalPinned = pinned;
        
        // Update unpin button visibility (pin button is handled by float panel)
        if (m_unpinButton) {
            m_unpinButton->Show(pinned); // Show unpin button when pinned
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



// FloatPanel methods implementation
void FlatUIBar::ShowPageInFloatPanel(FlatUIPage* page)
{
    if (!m_floatPanel || !page) {
        return;
    }

    wxWindow* frame = wxGetTopLevelParent(this);
    if (!frame) return;

    // Calculate position and size as per requirements
    // Position (0, 30) relative to the frame, converted to screen coordinates
    wxPoint position = frame->ClientToScreen(wxPoint(2, 32)); 
    
    // Size: frame's width and fixed height of 60
    wxSize size(frame->GetClientSize().GetWidth()-4, 60);

    if (m_floatPanel->IsShown()) {
        // Float panel is already shown, just update the content
        LOG_INF("Updating float panel content to: " + page->GetLabel().ToStdString(), "FlatUIBar");
        m_floatPanel->SetPageContent(page);
    } else {
        // Float panel is not shown, show it with the new content
        LOG_INF("Showing float panel with: " + page->GetLabel().ToStdString(), "FlatUIBar");
        m_floatPanel->SetPageContent(page);
        m_floatPanel->ShowAt(position, size);
    }

    // Ensure the activeFloatingPage is set correctly if it wasn't set by the caller
    if (m_activeFloatingPage == wxNOT_FOUND) {
        for (size_t i = 0; i < m_pages.size(); ++i) {
            if (m_pages[i] == page) {
                m_activeFloatingPage = i;
                break;
            }
        }
    }
}

void FlatUIBar::HideFloatPanel()
{
    if (m_floatPanel && m_floatPanel->IsShown()) {
        m_floatPanel->HidePanel();
        m_activeFloatingPage = wxNOT_FOUND; // Reset floating page selection
        Refresh(); // Update tab visual state
        LOG_INF("Hidden float panel", "FlatUIBar");
    }
}

void FlatUIBar::OnFloatPanelDismissed(wxCommandEvent& event)
{
    // Handle the event when the float panel is dismissed
    LOG_INF("Float panel dismissed, resetting active floating page", "FlatUIBar");
    m_activeFloatingPage = wxNOT_FOUND; // Reset floating page selection
    Refresh(); // Update tab visual state
    event.Skip();
}

