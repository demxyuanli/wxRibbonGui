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
#include "flatui/FlatUIConstants.h"
#include "config/ConstantsConfig.h"

#define CFG_COLOUR(key, def) ConstantsConfig::getInstance().getColourValue(key, def)
#define CFG_INT(key, def)    ConstantsConfig::getInstance().getIntValue(key, def)

int FlatUIBar::GetBarHeight()
{
    return CFG_INT("BarRenderHeight", FLATUI_BAR_RENDER_HEIGHT);
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
    m_tabStyle(TabStyle::DEFAULT),
    m_tabBorderStyle(TabBorderStyle::SOLID),
    m_tabBorderTop(2),
    m_tabBorderBottom(0),
    m_tabBorderLeft(0),
    m_tabBorderRight(0),
    m_tabCornerRadius(0),
    m_functionSpaceCenterAlign(false),
    m_profileSpaceRightAlign(false)
{
    SetFont(GetFlatUIDefaultFont());
    auto& cfg = ConstantsConfig::getInstance();
    m_tabBorderColour = CFG_COLOUR("BarTabBorderColour", FLATUI_BAR_TAB_BORDER_COLOUR);
    m_tabBorderTopColour = CFG_COLOUR("BarActiveTabTopBorderColour", FLATUI_BAR_ACTIVE_TAB_TOP_BORDER_COLOUR);
    m_tabBorderBottomColour = CFG_COLOUR("BarTabBorderColour", FLATUI_BAR_TAB_BORDER_COLOUR);
    m_tabBorderLeftColour = CFG_COLOUR("BarTabBorderColour", FLATUI_BAR_TAB_BORDER_COLOUR);
    m_tabBorderRightColour = CFG_COLOUR("BarTabBorderColour", FLATUI_BAR_TAB_BORDER_COLOUR);
    m_activeTabBgColour = CFG_COLOUR("ActBarBackgroundColour", FLATUI_PRIMARY_CONTENT_BG_COLOUR);
    m_activeTabTextColour = CFG_COLOUR("BarActiveTextColour", FLATUI_BAR_ACTIVE_TEXT_COLOUR);
    m_inactiveTabTextColour = CFG_COLOUR("BarInactiveTextColour", FLATUI_BAR_INACTIVE_TEXT_COLOUR);
    m_barTopMargin = CFG_INT("BarTopMargin", FLATUI_BAR_TOP_MARGIN);
    m_barBottomMargin = CFG_INT("BarTopMargin", FLATUI_BAR_TOP_MARGIN);

#ifdef __WXMSW__
    HWND hwnd = (HWND)GetHandle();
    if (hwnd) {
        long exStyle = ::GetWindowLong(hwnd, GWL_EXSTYLE);
        ::SetWindowLong(hwnd, GWL_EXSTYLE, exStyle | WS_EX_COMPOSITED);
    }
#endif

    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    // Create child component controls
    m_homeSpace = new FlatUIHomeSpace(this, wxID_ANY);
    m_functionSpace = new FlatUIFunctionSpace(this, wxID_ANY);
    m_profileSpace = new FlatUIProfileSpace(this, wxID_ANY);
    m_systemButtons = new FlatUISystemButtons(this, wxID_ANY);

    m_homeSpace->SetDoubleBuffered(true);
    m_functionSpace->SetDoubleBuffered(true);
    m_profileSpace->SetDoubleBuffered(true);
    m_systemButtons->SetDoubleBuffered(true);

    m_tabFunctionSpacer = new FlatUISpacerControl(this, 10);
    m_functionProfileSpacer = new FlatUISpacerControl(this, 10);
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

    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }

    // Ensure the initial page can be activated and displayed after all pages are added
    Bind(wxEVT_SHOW, [this](wxShowEvent& event) {
        if (event.IsShown() && m_activePage < m_pages.size() && m_pages[m_activePage]) {
            // Use timer instead of wxCallAfter
            wxTimer* timer = new wxTimer(this);
            Bind(wxEVT_TIMER, [this, timer](wxTimerEvent&) {
                if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
                    FlatUIPage* currentPage = m_pages[m_activePage];
                    currentPage->SetActive(true);
                    currentPage->Show();

                    wxSize barClientSize = GetClientSize();
                    int barStripHeight = GetBarHeight();
                    currentPage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));

                    int pageHeight = barClientSize.GetHeight() - barStripHeight - m_barTopMargin;
                    if (pageHeight < 0) {
                        pageHeight = 0;
                    }

                    currentPage->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));
                    currentPage->Layout();
                    currentPage->Refresh();

                    UpdateElementPositionsAndSizes(GetClientSize());
                    Refresh();
                }
                delete timer; // Clean up the timer
                }, timer->GetId());

            timer->StartOnce(50); // 50ms delay
        }
        event.Skip();
        });
}

FlatUIBar::~FlatUIBar()
{
}

wxSize FlatUIBar::DoGetBestSize() const
{
    wxSize bestSize(0, 0);
    bestSize.SetHeight(GetBarHeight() + m_barTopMargin); // Include top margin

    // Add the height of the active page, if any
    if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
        FlatUIPage* currentPage = m_pages[m_activePage];
        wxSize pageSize = currentPage->GetBestSize(); // Assuming FlatUIPage also implements GetBestSize or similar
        bestSize.SetHeight(bestSize.GetHeight() + pageSize.GetHeight());
        // The width of the FlatUIBar should ideally be determined by its contents or parent sizer.
        // For now, let's take the page's width as a hint, but this might need refinement.
        // If pages can have varying widths, the widest page or a default width might be better.
        bestSize.SetWidth(wxMax(bestSize.GetWidth(), pageSize.GetWidth()));
    }

    // The width calculation can be complex as it depends on home button, tabs, function/profile spaces, and system buttons.
    // A simple approach for now: Use a reasonable default or calculate based on visible elements.
    // For a more accurate width, we would need logic similar to UpdateElementPositionsAndSizes.
    // Let's use a default minimum width for now if no page is active or page has no width.
    if (bestSize.GetWidth() <= 0) {
        bestSize.SetWidth(wxWindow::DoGetBestSize().GetWidth()); // Fallback to default wxControl best width
        if (bestSize.GetWidth() <= 0) { // If still no width, use a sensible minimum
            bestSize.SetWidth(200); // Example: a minimum reasonable width for a bar
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
        // Ensure the first page is properly activated
        page->SetActive(true);
    }
    else {
        // Non-first pages are inactive by default
        page->SetActive(false);
    }

    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::SetActivePage(size_t index)
{
    if (index >= m_pages.size() || index == m_activePage)
        return;

    // Deactivate the previously active page
    if (m_activePage < m_pages.size() && m_pages[m_activePage]) {
        m_pages[m_activePage]->SetActive(false);
        m_pages[m_activePage]->Hide();
    }

    m_activePage = index;

    // Activate the new page
    FlatUIPage* currentPage = m_pages[m_activePage];
    if (currentPage) {
        // Set the page position and size
        wxSize barClientSize = GetClientSize();
        int barStripHeight = GetBarHeight();
        currentPage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));

        int pageHeight = barClientSize.GetHeight() - barStripHeight - m_barTopMargin;
        if (pageHeight < 0) {
            pageHeight = 0;
        }

        currentPage->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));

        // Make the page visible before setting it active to ensure layout calculations work
        currentPage->Show();

        // Activate the page and ensure all panels become visible
        currentPage->SetActive(true);

        // Force layout update to ensure proper positioning
        currentPage->Layout();
        currentPage->Refresh();

        Refresh();
    }
}

size_t FlatUIBar::GetPageCount() const { return m_pages.size(); }
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

