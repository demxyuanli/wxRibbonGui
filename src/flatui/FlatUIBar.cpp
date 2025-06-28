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
#include "flatui/FlatUIFixPanel.h"
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
    m_fixPanel(nullptr),
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

    // Create the fixed panel for containing pages (pin button is now handled by float panel)
    m_fixPanel = new FlatUIFixPanel(this, wxID_ANY);
    m_fixPanel->SetName("FixPanel");
    m_fixPanel->SetDoubleBuffered(true);
    m_fixPanel->Show(m_isGlobalPinned); // Show only when pinned initially
    
    // Get reference to unpin button from fix panel
    m_unpinButton = m_fixPanel->GetUnpinButton();
    LOG_INF("Created fix panel with unpin button, initially " + std::string(m_isGlobalPinned ? "shown" : "hidden"), "FlatUIBar");

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

    // Initialize panels based on initial pin state
    // Initial state is pinned (m_isGlobalPinned = true), so create fix panel
    if (m_isGlobalPinned) {
        // Create fix panel for initial pinned state
        m_fixPanel = new FlatUIFixPanel(this, wxID_ANY);
        m_fixPanel->SetName("FixPanel");
        m_fixPanel->SetDoubleBuffered(true);
        m_fixPanel->Show(true);
        
        // Get reference to unpin button from fix panel
        m_unpinButton = m_fixPanel->GetUnpinButton();
        
        // Bind the unpin button event
        Bind(wxEVT_UNPIN_BUTTON_CLICKED, &FlatUIBar::OnUnpinButtonClicked, this, m_unpinButton->GetId());
        
        LOG_INF("Constructor: Created FixPanel for initial pinned state", "FlatUIBar");
    } else {
        // Create float panel for unpinned state
        m_floatPanel = new FlatUIFloatPanel(this);
        
        // Bind pin button events from float panel
        Bind(wxEVT_PIN_BUTTON_CLICKED, &FlatUIBar::OnPinButtonClicked, this);
        
        // Bind float panel dismiss event
        Bind(wxEVT_FLOAT_PANEL_DISMISSED, &FlatUIBar::OnFloatPanelDismissed, this);
        
        LOG_INF("Constructor: Created FloatPanel for initial unpinned state", "FlatUIBar");
    }

    // Setup global mouse capture
    SetupGlobalMouseCapture();

    // Always initialize layout, regardless of visibility state
    CallAfter([this]() {
        UpdateElementPositionsAndSizes(GetClientSize());
        if (IsShown()) {
            Refresh();
        }
        LOG_INF("Constructor: Completed initial layout setup", "FlatUIBar");
    });

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

    if (m_fixPanel) {
        m_fixPanel->Destroy();
        m_fixPanel = nullptr;
    }

    // Clear all pages (they are now managed by FixPanel)
    m_pages.clear();
}

void FlatUIBar::OnShow(wxShowEvent& event)
{
    if (event.IsShown()) {
        CallAfter([this]() {
            LOG_INF("OnShow: Processing show event for FlatUIBar", "FlatUIBar");
            
            // Always call UpdateElementPositionsAndSizes to ensure proper layout
            UpdateElementPositionsAndSizes(GetClientSize());
            
            // Show logic considering pinned and temporarily shown states
            if (m_isGlobalPinned && m_fixPanel) {
                // Show fix panel and set active page
                if (!m_fixPanel->IsShown()) {
                    m_fixPanel->Show();
                    LOG_INF("OnShow: Showed FixPanel in pinned state", "FlatUIBar");
                }
                
                // Set active page in fix panel if there are pages
                if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
                    m_fixPanel->SetActivePage(m_activePage);
                    LOG_INF("OnShow: Set active page in FixPanel", "FlatUIBar");
                }
            }
            else if (!m_isGlobalPinned && m_fixPanel) {
                // If not pinned but fix panel exists, ensure it's hidden
                if (m_fixPanel->IsShown()) {
                    m_fixPanel->Hide();
                    LOG_INF("OnShow: Hidden FixPanel in unpinned state", "FlatUIBar");
                }
            }

            // Update button visibility after showing/hiding panels
            UpdateButtonVisibility();

            Refresh();
            });
    }
    event.Skip();
}

wxSize FlatUIBar::DoGetBestSize() const
{
    wxSize bestSize(0, 0);

    // Determine if we need to show pages, which dictates the total height.
    if (ShouldShowPages()) {
        // Pinned, or unpinned with a temporary page: calculate full size.
        bestSize.SetHeight(GetBarHeight() + m_barTopMargin); 

        if (m_fixPanel && m_fixPanel->IsShown()) {
            // Add the height of the fix panel.
            wxSize fixPanelSize = m_fixPanel->GetBestSize();
            bestSize.SetHeight(bestSize.GetHeight() + fixPanelSize.GetHeight());
            bestSize.SetWidth(wxMax(bestSize.GetWidth(), fixPanelSize.GetWidth()));
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

    // Add page to vector
    m_pages.push_back(page);

    // Add page to fix panel if it exists (pinned state)
    if (m_fixPanel) {
        m_fixPanel->AddPage(page);
        
        // Set active page in fix panel
        if (m_pages.size() == 1) {
            m_activePage = 0;
            m_fixPanel->SetActivePage(static_cast<size_t>(0));
            page->SetActive(true);
        }
        else {
            page->SetActive(false);
        }
        
        LOG_INF("Added page '" + page->GetLabel().ToStdString() + "' to FlatUIBar via FixPanel", "FlatUIBar");
    } else {
        // Unpinned state - pages are managed directly by FlatUIBar for float panel usage
        page->Hide(); // Initially hidden
        
        if (m_pages.size() == 1) {
            m_activePage = 0;
            page->SetActive(true);
        }
        else {
            page->SetActive(false);
        }
        
        LOG_INF("Added page '" + page->GetLabel().ToStdString() + "' to FlatUIBar for FloatPanel usage", "FlatUIBar");
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

    if (m_activePage == pageIndex && m_isGlobalPinned && m_fixPanel && m_fixPanel->GetActivePage() == m_pages[pageIndex]) {
        return; // Already active and shown in pinned state, do nothing.
    }

    m_activePage = pageIndex;
    FlatUIPage* newPage = m_pages[m_activePage];

    // Update the fix panel's active page if in pinned state and fix panel exists
    if (m_isGlobalPinned && m_fixPanel) {
        m_fixPanel->SetActivePage(static_cast<size_t>(pageIndex));
        m_temporarilyShownPage = nullptr; // Ensure no temporary page is set
        
        // Only trigger layout update if needed, not full parent layout
        UpdateElementPositionsAndSizes(GetClientSize());
    }
    
    Refresh();

    LOG_INF("Set active page to '" + newPage->GetLabel().ToStdString() + "' (index " + std::to_string(pageIndex) + ")", "FlatUIBar");
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
    LOG_INF("OnPinButtonClicked: Pin button clicked, switching to pinned state", "FlatUIBar");
    
    // Step 1: Destroy float panel
    if (m_floatPanel) {
        LOG_INF("OnPinButtonClicked: Destroying float panel", "FlatUIBar");
        m_floatPanel->HidePanel();
        m_floatPanel->Destroy();
        m_floatPanel = nullptr;
    }
    
    // Step 2: Ensure fix panel exists and is positioned correctly
    if (!m_fixPanel) {
        LOG_INF("OnPinButtonClicked: Creating new fix panel", "FlatUIBar");
        m_fixPanel = new FlatUIFixPanel(this, wxID_ANY);
        m_fixPanel->SetName("FixPanel");
        m_fixPanel->SetDoubleBuffered(true);
        
        // Re-add all pages to the new fix panel
        for (auto* page : m_pages) {
            if (page) {
                m_fixPanel->AddPage(page);
            }
        }
        
        // Set the correct active page in the fix panel
        if (m_activePage < m_pages.size()) {
            m_fixPanel->SetActivePage(m_activePage);
            LOG_INF("OnPinButtonClicked: Set active page " + std::to_string(m_activePage) + " in new FixPanel", "FlatUIBar");
        }
        
        // Get reference to unpin button from fix panel
        m_unpinButton = m_fixPanel->GetUnpinButton();
        
        // Bind the unpin button event
        Bind(wxEVT_UNPIN_BUTTON_CLICKED, &FlatUIBar::OnUnpinButtonClicked, this, m_unpinButton->GetId());
    }
    
    SetGlobalPinned(true);
    
    // Send the old compatible event for FlatFrame
    wxCommandEvent pinEvent(wxEVT_PIN_STATE_CHANGED, GetId());
    pinEvent.SetEventObject(this);
    pinEvent.SetInt(1); // pinned = true
    GetParent()->GetEventHandler()->ProcessEvent(pinEvent);
    
    LOG_INF("OnPinButtonClicked: Completed pin operation", "FlatUIBar");
    event.Skip();
}

void FlatUIBar::OnUnpinButtonClicked(wxCommandEvent& event)
{
    LOG_INF("OnUnpinButtonClicked: Unpin button clicked, switching to unpinned state", "FlatUIBar");
    
    // Step 1: m_activePage will be preserved through the transition
    
    // Step 2: Unbind unpin button event before destroying fix panel
    if (m_unpinButton) {
        Unbind(wxEVT_UNPIN_BUTTON_CLICKED, &FlatUIBar::OnUnpinButtonClicked, this, m_unpinButton->GetId());
        m_unpinButton = nullptr; // Will be destroyed with fix panel
    }
    
    // Step 3: Return pages to original parent before destroying fix panel
    if (m_fixPanel) {
        LOG_INF("OnUnpinButtonClicked: Returning pages to original parent", "FlatUIBar");
        for (auto* page : m_pages) {
            if (page) {
                page->Reparent(this);
                page->Hide();
            }
        }
        
        LOG_INF("OnUnpinButtonClicked: Destroying fix panel", "FlatUIBar");
        m_fixPanel->Destroy();
        m_fixPanel = nullptr;
    }
    
    // Step 4: Create new float panel
    if (!m_floatPanel) {
        LOG_INF("OnUnpinButtonClicked: Creating new float panel", "FlatUIBar");
        m_floatPanel = new FlatUIFloatPanel(this);
        
        // Bind pin button events from float panel
        Bind(wxEVT_PIN_BUTTON_CLICKED, &FlatUIBar::OnPinButtonClicked, this);
        
        // Bind float panel dismiss event
        Bind(wxEVT_FLOAT_PANEL_DISMISSED, &FlatUIBar::OnFloatPanelDismissed, this);
    }
    
    SetGlobalPinned(false);
    
    // m_activePage is preserved and will be used when switching back to pinned state
    
    // Send the old compatible event for FlatFrame
    wxCommandEvent pinEvent(wxEVT_PIN_STATE_CHANGED, GetId());
    pinEvent.SetEventObject(this);
    pinEvent.SetInt(0); // pinned = false
    GetParent()->GetEventHandler()->ProcessEvent(pinEvent);
    
    LOG_INF("OnUnpinButtonClicked: Completed unpin operation", "FlatUIBar");
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
        
        LOG_INF("SetGlobalPinned: Changing from " + std::string(m_isGlobalPinned ? "pinned" : "unpinned") + 
                " to " + std::string(pinned ? "pinned" : "unpinned"), "FlatUIBar");
        
        if (!pinned) { // Transitioning to UNPINNED
            m_lastActivePageBeforeUnpin = m_activePage;
            // Keep m_activePage as is - don't reset it to wxNOT_FOUND
            // This allows seamless transitions back to pinned state
            m_activeFloatingPage = wxNOT_FOUND; // Reset floating page selection initially
            
        } else { // Transitioning to PINNED
            // Ensure float panel is hidden when pinning
            HideFloatPanel();
            
            // m_activePage should already be correct from float panel interactions
            // Just validate and provide fallbacks if needed
            if (m_activePage >= m_pages.size() || m_activePage == wxNOT_FOUND) {
                if (!m_pages.empty()) {
                    m_activePage = 0; // Fallback to first page
                    LOG_INF("SetGlobalPinned: Invalid active page, fallback to first page", "FlatUIBar");
                } else {
                    m_activePage = wxNOT_FOUND; // No pages available
                    LOG_INF("SetGlobalPinned: No pages available", "FlatUIBar");
                }
            } else {
                LOG_INF("SetGlobalPinned: Using current active page " + std::to_string(m_activePage) + " for pinned state", "FlatUIBar");
            }
            
            m_activeFloatingPage = wxNOT_FOUND; // Reset floating page selection
        }
        
        m_isGlobalPinned = pinned;
        
        // Update button visibility based on new state
        UpdateButtonVisibility();
        
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
    UpdateElementPositionsAndSizes(GetClientSize());
    InvalidateBestSize();
    wxWindow* parent = GetParent();
    if (parent) {
        parent->InvalidateBestSize();
        parent->Layout();
    }
    Refresh();
}

void FlatUIBar::ShowAllContent()
{
    LOG_INF("ShowAllContent: Showing all page content via FixPanel", "FlatUIBar");
    
    // Show the fix panel and set active page
    if (m_fixPanel) {
        if (!m_fixPanel->IsShown()) {
            m_fixPanel->Show();
            LOG_INF("ShowAllContent: Showed FixPanel", "FlatUIBar");
        }

        // Set active page in fix panel
        if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
            m_fixPanel->SetActivePage(m_activePage);
            LOG_INF("ShowAllContent: Set active page in FixPanel - " + m_pages[m_activePage]->GetLabel().ToStdString(), "FlatUIBar");
        }

        LOG_INF("ShowAllContent: FixPanel shown, positioning will be handled by UpdateElementPositionsAndSizes", "FlatUIBar");
    }
    
    // Update button visibility after showing fix panel
    UpdateButtonVisibility();

    // Clear temporarily shown page state since we're in pinned mode
    m_temporarilyShownPage = nullptr;
    LOG_INF("ShowAllContent: Cleared temporarily shown page state", "FlatUIBar");
}

void FlatUIBar::HideAllContentExceptBarSpace()
{
    LOG_INF("HideAllContentExceptBarSpace: Hiding all page content via FixPanel", "FlatUIBar");
    
    // Hide the fix panel (which contains all pages)
    if (m_fixPanel && m_fixPanel->IsShown()) {
        m_fixPanel->Hide();
        LOG_INF("HideAllContentExceptBarSpace: Hidden FixPanel", "FlatUIBar");
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

    // Calculate position and size to match FixPanel positioning
    // Position (0, 30) relative to this bar, converted to screen coordinates
    const int FLOAT_PANEL_Y = 30;
    wxPoint position = this->ClientToScreen(wxPoint(0, FLOAT_PANEL_Y)); 
    
    // Size: bar's width and calculated height based on page content
    wxSize pageSize = page->GetBestSize();
    int floatHeight = wxMax(60, pageSize.GetHeight()); // Minimum 60px height
    wxSize size(GetClientSize().GetWidth(), floatHeight);

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

void FlatUIBar::UpdateButtonVisibility()
{
    LOG_INF("UpdateButtonVisibility: Current state is " + std::string(m_isGlobalPinned ? "pinned" : "unpinned"), "FlatUIBar");
    
    // Unpin button logic: Show only when ribbon is pinned and fix panel is shown
    if (m_fixPanel && m_unpinButton) {
        bool shouldShowUnpin = m_isGlobalPinned && m_fixPanel->IsShown();
        m_fixPanel->ShowUnpinButton(shouldShowUnpin);
        LOG_INF("UpdateButtonVisibility: " + std::string(shouldShowUnpin ? "Showed" : "Hidden") + " unpin button in FixPanel", "FlatUIBar");
    }
    
    // Pin button logic: Handled by float panel, should be hidden when ribbon is pinned
    // When ribbon is unpinned, pin button will be shown by float panel when it's displayed
    if (m_floatPanel && m_floatPanel->IsShown()) {
        // Float panel is shown, pin button should be visible (handled by float panel)
        LOG_INF("UpdateButtonVisibility: Float panel is shown, pin button managed by float panel", "FlatUIBar");
    } else {
        // Float panel is hidden, ensure pin button is not visible
        LOG_INF("UpdateButtonVisibility: Float panel is hidden, pin button should be hidden", "FlatUIBar");
    }
    
    // Refresh to update visual state, positioning is handled by UpdateElementPositionsAndSizes
    if (IsShown()) {
        Refresh();
    }
}

