#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIPage.h" // For m_pages vector
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIHomeSpace.h"
#include "flatui/FlatUIFunctionSpace.h"
#include "flatui/FlatUIProfileSpace.h"
#include "flatui/FlatUISystemButtons.h"
#include "flatui/FlatUIEventManager.h" 
#include "flatui/FlatUISpacerControl.h"
#include <string>            // For std::to_string
#include <numeric>           // For std::accumulate if calculating total tab width
#include <wx/dcbuffer.h>     // For wxAutoBufferedPaintDC
#include <wx/timer.h>        // For wxTimer instead of wxCallAfter
#include <logger/Logger.h>     // For wxGraphicsContext
#include "flatui/FlatUIConstants.h" // Include for font constants

// Height of the entire FlatUIBar strip

// Static method implementation
int FlatUIBar::GetBarHeight() 
{
    return FLATUI_BAR_RENDER_HEIGHT;
}

FlatUIBar::FlatUIBar(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : wxControl(parent, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE), 
      m_activePage(0), // Initialize m_activePage first
      m_homeSpace(nullptr),
      m_functionSpace(nullptr),
      m_profileSpace(nullptr),
      m_systemButtons(nullptr),
      m_tabFunctionSpacer(nullptr),
      m_functionProfileSpacer(nullptr)
{
    SetFont(GetFlatUIDefaultFont());

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
    
    m_tabFunctionSpacer = new FlatUISpacerControl(this, 0);
    m_functionProfileSpacer = new FlatUISpacerControl(this, 0);
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
                    currentPage->SetPosition(wxPoint(0, barStripHeight));
                    
                    int pageHeight = barClientSize.GetHeight() - barStripHeight;
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
    bestSize.SetHeight(GetBarHeight()); // Start with the height of the tab bar itself

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
    
    // Ensure minimum height is at least the bar height
    if (bestSize.GetHeight() < GetBarHeight()) {
        bestSize.SetHeight(GetBarHeight());
    }

    return bestSize;
}

// --- Configuration Method Implementations ---
void FlatUIBar::SetHomeButtonMenu(wxMenu* menu)
{
    // if (m_homeSpace) m_homeSpace->SetMenu(menu); // Removed as FlatUIHomeSpace now uses FlatUIHomeMenu internally
}

void FlatUIBar::SetHomeButtonIcon(const wxBitmap& icon)
{
    if (m_homeSpace) m_homeSpace->SetIcon(icon);
}

void FlatUIBar::SetHomeButtonWidth(int width)
{
    if (m_homeSpace && width > 0) {
        m_homeSpace->SetButtonWidth(width);
        if (IsShown()) Layout(); // Trigger re-layout of FlatUIBar if visible
    }
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
    } else {
        // Non-first pages are inactive by default
        page->SetActive(false);
    } 
    
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
        
        LOG_INF("Added page '" + page->GetLabel().ToStdString() + "', total pages: " + std::to_string(m_pages.size()) + ", active page: " + std::to_string(m_activePage), "FlatUIBar");
    }
}

void FlatUIBar::SetActivePage(size_t index)
{
    if (index >= m_pages.size() || index == m_activePage)
        return;

    LOG_INF("Setting active page to index: " + std::to_string(index), "FlatUIBar");

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
        currentPage->SetPosition(wxPoint(0, barStripHeight));
            
            int pageHeight = barClientSize.GetHeight() - barStripHeight;
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
        
        // Log activation for debugging
        LOG_DBG("Page activated: " + currentPage->GetLabel().ToStdString() +
            ", Panels: " + std::to_string(currentPage->GetPanels().size()),
            "FlatUIBar");
        }
    
    Refresh();
}

size_t FlatUIBar::GetPageCount() const { return m_pages.size(); }
FlatUIPage* FlatUIBar::GetPage(size_t index) const { return (index < m_pages.size()) ? m_pages[index] : nullptr; }


void FlatUIBar::SetFunctionSpaceControl(wxWindow* funcControl, int width)
{
    if (m_functionSpace) {
        m_functionSpace->SetChildControl(funcControl);
        if (width > 0) m_functionSpace->SetSpaceWidth(width);
        m_functionSpace->Show(funcControl != nullptr);
        if (IsShown()) Layout(); // Trigger re-layout of FlatUIBar
    }
}

void FlatUIBar::SetProfileSpaceControl(wxWindow* profControl, int width)
{
    if (m_profileSpace) {
        m_profileSpace->SetChildControl(profControl);
        if (width > 0) m_profileSpace->SetSpaceWidth(width);
        m_profileSpace->Show(profControl != nullptr);
        if (IsShown()) Layout(); // Trigger re-layout of FlatUIBar
    }
}


// CalculateTabsWidth remains for direct tab drawing
int FlatUIBar::CalculateTabsWidth(wxDC& dc) const
{
    int totalWidth = 0;
    if (m_pages.empty()) return 0; 
    for (size_t i = 0; i < m_pages.size(); ++i)
    {
        if (!m_pages[i]) continue;
        wxString label = m_pages[i]->GetLabel();
        wxSize labelSize = dc.GetTextExtent(label);
        totalWidth += labelSize.GetWidth() + FLATUI_BAR_TAB_PADDING * 2;
        if (i < m_pages.size() - 1)
        {
            totalWidth += FLATUI_BAR_TAB_SPACING;
        }
    }
    return totalWidth;
}

void FlatUIBar::UpdateElementPositionsAndSizes(const wxSize& barClientSz)
{
    if (!m_homeSpace || !m_systemButtons || !m_functionSpace || !m_profileSpace) {
        LOG_DBG("FlatUIBar::UpdateElementPositionsAndSizes - one or more child components are null.","FlatUIBar");
        return; // Components not ready
    }

    wxClientDC dc(this); 
    int currentX = FLATUI_BAR_BAR_PADDING;
    int barStripHeight = GetBarHeight();
    int elementY = 0;

    // 1. HomeSpace
    int homeActualWidth = 0; 
    if (m_homeSpace && m_homeSpace->IsShown()) { 
        homeActualWidth = m_homeSpace->GetButtonWidth(); 
    }
    m_homeSpace->SetPosition(wxPoint(currentX, elementY));
    m_homeSpace->SetSize(homeActualWidth, barStripHeight);
   
    if (homeActualWidth > 0) {
        m_homeSpace->Show(true);
        currentX += homeActualWidth + FLATUI_BAR_ELEMENT_SPACING;
    } else {
        m_homeSpace->Show(false);
    }

    int sysButtonsWidth = 0;
    if (m_systemButtons && m_systemButtons->IsShown()) {
        sysButtonsWidth = m_systemButtons->GetMyRequiredWidth();
    }

    int rightBoundary = barClientSz.GetWidth() - FLATUI_BAR_BAR_PADDING;
    if (sysButtonsWidth > 0) {
        rightBoundary -= (sysButtonsWidth + FLATUI_BAR_ELEMENT_SPACING);
    }

    bool tabFuncSpacerVisible = m_tabFunctionSpacer && m_tabFunctionSpacer->IsShown();
    bool tabFuncSpacerAutoExpand = tabFuncSpacerVisible && m_tabFunctionSpacer->GetAutoExpand();

    bool funcProfileSpacerVisible = m_functionProfileSpacer && m_functionProfileSpacer->IsShown();
    bool funcProfileSpacerAutoExpand = funcProfileSpacerVisible && m_functionProfileSpacer->GetAutoExpand();

    int funcRequestedWidth = 0; 
    bool funcSpaceIsEffectivelyVisible = m_functionSpace && m_functionSpace->IsShown() && m_functionSpace->GetChildControl(); 
    if (funcSpaceIsEffectivelyVisible) {
        funcRequestedWidth = m_functionSpace->GetSpaceWidth(); 
    }

    int profileRequestedWidth = 0; 
    bool profileSpaceIsEffectivelyVisible = m_profileSpace && m_profileSpace->IsShown() && m_profileSpace->GetChildControl(); 
    if (profileSpaceIsEffectivelyVisible) {
        profileRequestedWidth = m_profileSpace->GetSpaceWidth(); 
    }

    if (tabFuncSpacerAutoExpand || funcProfileSpacerAutoExpand) {
        int tabsNeededWidth = CalculateTabsWidth(dc);
        
        if (tabsNeededWidth > 0) {
            m_tabAreaRect = wxRect(currentX, elementY, tabsNeededWidth, barStripHeight);
            currentX += tabsNeededWidth + FLATUI_BAR_ELEMENT_SPACING;
        } else {
            m_tabAreaRect = wxRect();
        }
        
        if (tabFuncSpacerAutoExpand && funcSpaceIsEffectivelyVisible) {
            int reservedWidthAfterTabFunc = 0;
            
            reservedWidthAfterTabFunc += funcRequestedWidth;
            
            if (profileSpaceIsEffectivelyVisible) {
                reservedWidthAfterTabFunc += profileRequestedWidth;
                
                if (funcProfileSpacerVisible && !funcProfileSpacerAutoExpand) {
                    reservedWidthAfterTabFunc += m_functionProfileSpacer->GetSpacerWidth();
                } else if (!funcProfileSpacerVisible) {
                    reservedWidthAfterTabFunc += FLATUI_BAR_ELEMENT_SPACING;
                }
            }

            int autoSpacerWidth = rightBoundary - currentX - reservedWidthAfterTabFunc;
            autoSpacerWidth = wxMax(m_tabFunctionSpacer->GetSpacerWidth(), autoSpacerWidth);
            
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(autoSpacerWidth, barStripHeight);
            currentX += autoSpacerWidth;
            
            m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, barStripHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
            if (profileSpaceIsEffectivelyVisible) {
                if (funcProfileSpacerVisible) {
                    if (funcProfileSpacerAutoExpand) {
                        int remainingSpace = rightBoundary - currentX - profileRequestedWidth;
                        remainingSpace = wxMax(m_functionProfileSpacer->GetSpacerWidth(), remainingSpace);
                        
                        m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
                        m_functionProfileSpacer->SetSize(remainingSpace, barStripHeight);
                        currentX += remainingSpace;
                    } else {
                        m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
                        m_functionProfileSpacer->SetSize(m_functionProfileSpacer->GetSpacerWidth(), barStripHeight);
                        currentX += m_functionProfileSpacer->GetSpacerWidth();
                    }
                } else {
                    currentX += FLATUI_BAR_ELEMENT_SPACING;
                }
                m_profileSpace->SetPosition(wxPoint(currentX, elementY));
                m_profileSpace->SetSize(profileRequestedWidth, barStripHeight);
                m_profileSpace->Show(true);
            } else {
                m_profileSpace->Show(false);
            }
        }
        else if (funcProfileSpacerAutoExpand && funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            if (tabFuncSpacerVisible) {
                m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
                m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), barStripHeight);
                currentX += m_tabFunctionSpacer->GetSpacerWidth();
            } else {
                currentX += FLATUI_BAR_ELEMENT_SPACING;
            }
            
            m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, barStripHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
            int autoSpacerWidth = rightBoundary - currentX - profileRequestedWidth;
            autoSpacerWidth = wxMax(m_functionProfileSpacer->GetSpacerWidth(), autoSpacerWidth);
            m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            m_functionProfileSpacer->SetSize(autoSpacerWidth, barStripHeight);
            currentX += autoSpacerWidth;
            m_profileSpace->SetPosition(wxPoint(currentX, elementY));
            m_profileSpace->SetSize(profileRequestedWidth, barStripHeight);
            m_profileSpace->Show(true);
        }
        else {
            int tabsNeededWidth = CalculateTabsWidth(dc);
            int availableWidthForTabs = rightBoundary - currentX;
            
            int reservedWidth = 0;
            
            if (funcSpaceIsEffectivelyVisible) {
                reservedWidth += funcRequestedWidth;
                if (tabFuncSpacerVisible) {
                    reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
                } else {
                    reservedWidth += FLATUI_BAR_ELEMENT_SPACING;
                }
            }
            
            if (profileSpaceIsEffectivelyVisible) {
                reservedWidth += profileRequestedWidth;
                if (funcSpaceIsEffectivelyVisible) {
                    if (funcProfileSpacerVisible) {
                        reservedWidth += m_functionProfileSpacer->GetSpacerWidth();
                    } else {
                        reservedWidth += FLATUI_BAR_ELEMENT_SPACING;
                    }
                } else {
                    if (tabFuncSpacerVisible) {
                        reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
                    } else {
                        reservedWidth += FLATUI_BAR_ELEMENT_SPACING;
            }
        }
    }
    
            availableWidthForTabs = wxMax(0, availableWidthForTabs - reservedWidth);
            
            int tabsWidth = wxMin(tabsNeededWidth, availableWidthForTabs);
            
            if (tabsWidth > 0) {
                m_tabAreaRect = wxRect(currentX, elementY, tabsWidth, barStripHeight);
                currentX += tabsWidth;
            } else {
                m_tabAreaRect = wxRect();
            }
            
            if (tabFuncSpacerVisible && funcSpaceIsEffectivelyVisible) {
                m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
                m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), barStripHeight);
                currentX += m_tabFunctionSpacer->GetSpacerWidth();
            } else if (funcSpaceIsEffectivelyVisible) {
                currentX += FLATUI_BAR_ELEMENT_SPACING;
            }
            
            if (funcSpaceIsEffectivelyVisible) {
                m_functionSpace->SetPosition(wxPoint(currentX, elementY));
                m_functionSpace->SetSize(funcRequestedWidth, barStripHeight);
                m_functionSpace->Show(true);
                currentX += funcRequestedWidth;
            } else {
                m_functionSpace->Show(false);
            }
            
            if (funcProfileSpacerVisible && funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
                m_functionProfileSpacer->SetSize(m_functionProfileSpacer->GetSpacerWidth(), barStripHeight);
                currentX += m_functionProfileSpacer->GetSpacerWidth();
            } else if (funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                currentX += FLATUI_BAR_ELEMENT_SPACING;
            } else if (!funcSpaceIsEffectivelyVisible && tabFuncSpacerVisible && profileSpaceIsEffectivelyVisible) {
                m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
                m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), barStripHeight);
                currentX += m_tabFunctionSpacer->GetSpacerWidth();
            } else if (!funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                currentX += FLATUI_BAR_ELEMENT_SPACING;
            }
            
            if (profileSpaceIsEffectivelyVisible) {
                m_profileSpace->SetPosition(wxPoint(currentX, elementY));
                m_profileSpace->SetSize(profileRequestedWidth, barStripHeight);
                m_profileSpace->Show(true);
                currentX += profileRequestedWidth;
            } else {
                m_profileSpace->Show(false);
            }
        }
    }
    else {
        int tabsNeededWidth = CalculateTabsWidth(dc);
        int availableWidthForTabs = rightBoundary - currentX;
        
        int reservedWidth = 0;
        
        if (funcSpaceIsEffectivelyVisible) {
            reservedWidth += funcRequestedWidth;
            if (tabFuncSpacerVisible) {
                reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
            } else {
                reservedWidth += FLATUI_BAR_ELEMENT_SPACING;
            }
        }
        
        if (profileSpaceIsEffectivelyVisible) {
            reservedWidth += profileRequestedWidth;
            if (funcSpaceIsEffectivelyVisible) {
                if (funcProfileSpacerVisible) {
                    reservedWidth += m_functionProfileSpacer->GetSpacerWidth();
                } else {
                    reservedWidth += FLATUI_BAR_ELEMENT_SPACING;
                }
            } else {
                if (tabFuncSpacerVisible) {
                    reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
                } else {
                    reservedWidth += FLATUI_BAR_ELEMENT_SPACING;
                }
            }
        }
        
        availableWidthForTabs = wxMax(0, availableWidthForTabs - reservedWidth);
        
        int tabsWidth = wxMin(tabsNeededWidth, availableWidthForTabs);
        
        if (tabsWidth > 0) {
            m_tabAreaRect = wxRect(currentX, elementY, tabsWidth, barStripHeight);
            currentX += tabsWidth;
    } else {
            m_tabAreaRect = wxRect();
        }
        
        if (tabFuncSpacerVisible && funcSpaceIsEffectivelyVisible) {
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), barStripHeight);
            currentX += m_tabFunctionSpacer->GetSpacerWidth();
        } else if (funcSpaceIsEffectivelyVisible) {
            currentX += FLATUI_BAR_ELEMENT_SPACING;
        }
        
        if (funcSpaceIsEffectivelyVisible) {
        m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, barStripHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
    } else {
            m_functionSpace->Show(false);
        }
        
        if (funcProfileSpacerVisible && funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            m_functionProfileSpacer->SetSize(m_functionProfileSpacer->GetSpacerWidth(), barStripHeight);
            currentX += m_functionProfileSpacer->GetSpacerWidth();
        } else if (funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            currentX += FLATUI_BAR_ELEMENT_SPACING;
        } else if (!funcSpaceIsEffectivelyVisible && tabFuncSpacerVisible && profileSpaceIsEffectivelyVisible) {
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), barStripHeight);
            currentX += m_tabFunctionSpacer->GetSpacerWidth();
        } else if (!funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            currentX += FLATUI_BAR_ELEMENT_SPACING;
        }
        
        if (profileSpaceIsEffectivelyVisible) {
        m_profileSpace->SetPosition(wxPoint(currentX, elementY));
            m_profileSpace->SetSize(profileRequestedWidth, barStripHeight);
            m_profileSpace->Show(true);
            currentX += profileRequestedWidth;
    } else {
            m_profileSpace->Show(false);
        }
    }
    if (sysButtonsWidth > 0) {
         m_systemButtons->SetPosition(wxPoint(barClientSz.GetWidth() - FLATUI_BAR_BAR_PADDING - sysButtonsWidth, elementY));
        m_systemButtons->SetSize(sysButtonsWidth, barStripHeight);
        m_systemButtons->Show(true);
    } else {
        m_systemButtons->Show(false);
    }
    if (m_activePage < m_pages.size() && m_pages[m_activePage])
    {
        FlatUIPage* currentPage = m_pages[m_activePage];
        
        currentPage->SetPosition(wxPoint(0, barStripHeight));
        
        int pageHeight = barClientSz.GetHeight() - barStripHeight;
        if (pageHeight < 0) {
            pageHeight = 0;
            LOG_DBG("Warning: Page height calculated as negative, adjusted to 0.", "FlatUIBar");
        }
        currentPage->SetSize(barClientSz.GetWidth(), pageHeight);
        
        // Ensure the active page is correctly activated and displayed
        currentPage->SetActive(true);
        if (!currentPage->IsShown()) { 
            currentPage->Show();
        }
        
        currentPage->Layout();
        currentPage->Refresh();
    }
    for (size_t i = 0; i < m_pages.size(); ++i) {
        if (i != m_activePage && m_pages[i]) {
            m_pages[i]->SetActive(false);
            if (m_pages[i]->IsShown()) {
            m_pages[i]->Hide();
            }
        }
    }

    Refresh();
}

void FlatUIBar::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize clientSize = GetClientSize();

    // Ribbon style: Define specific colors
    // const wxColour barBackgroundColor(220, 225, 230); // Now from FlatUIConstants.h

    dc.SetBrush(FLATUI_BAR_BACKGROUND_COLOUR);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.Clear(); // Clear with the new bar background color
    // It's often good to draw a rectangle for the bar area if borders are desired,
    // but for a flatter look, Clear() might be sufficient if child controls define edges.
    // For now, we'll rely on PaintTabs and child controls to define the visual structure on top of this background.
    // dc.DrawRectangle(0, 0, clientSize.GetWidth(), clientSize.GetHeight()); // Optional: redraw rect if a border for the whole bar is needed

    if (!m_pages.empty() && m_tabAreaRect.GetWidth() > 0) {
        wxDCClipper clip(dc, m_tabAreaRect);
        int currentXOffsetForTabs = m_tabAreaRect.GetX();
        PaintTabs(dc, m_tabAreaRect.GetWidth(), currentXOffsetForTabs);
    }
}

void FlatUIBar::PaintTabs(wxDC& dc, int availableTotalWidth, int& currentXOffsetInOut)
{
    int tabYPos = 0; 
    int barEffectiveHeight = GetBarHeight(); 
    int initialXOffset = currentXOffsetInOut;

    dc.SetFont(GetFont()); 

    for (size_t i = 0; i < m_pages.size(); ++i)
    {
        if (!m_pages[i]) continue;

        FlatUIPage* page = m_pages[i];
        wxString label = page->GetLabel();
        wxSize labelSize = dc.GetTextExtent(label);
        int tabWidth = labelSize.GetWidth() + FLATUI_BAR_TAB_PADDING * 2; 

        if ((currentXOffsetInOut + tabWidth) > (initialXOffset + availableTotalWidth) && i > 0) { 
            break; 
        }

        wxRect tabRect(currentXOffsetInOut, tabYPos, tabWidth, barEffectiveHeight); 

        if (i == m_activePage)
        {
            dc.SetBrush(wxBrush(FLATUI_PRIMARY_CONTENT_BG_COLOUR)); 
            dc.SetTextForeground(FLATUI_BAR_ACTIVE_TEXT_COLOUR); 
            
            // Fill background of active tab (excluding the 2px top border for now)
            // We'll draw a slightly smaller rect for fill, then draw borders around it.
            // Or, fill all then draw borders on top. Let's try fill all then specific borders.
            dc.SetPen(*wxTRANSPARENT_PEN); // No pen for the main fill
            dc.DrawRectangle(tabRect.x, tabRect.y + 2, tabRect.width, tabRect.height - 2); // Fill below top border

            // Draw 2px top border
            dc.SetPen(wxPen(FLATUI_BAR_ACTIVE_TAB_TOP_BORDER_COLOUR, 2));
            dc.DrawLine(tabRect.GetLeft(), tabRect.GetTop() + 1, 
                        tabRect.GetRight() +1 , tabRect.GetTop() + 1); // +1 for right coord to cover full width

            // Draw 1px left border
            dc.SetPen(wxPen(FLATUI_BAR_TAB_BORDER_COLOUR, 1));
            dc.DrawLine(tabRect.GetLeft(), tabRect.GetTop() + 2, 
                        tabRect.GetLeft(), tabRect.GetBottom());

            // Draw 1px right border
            dc.SetPen(wxPen(FLATUI_BAR_TAB_BORDER_COLOUR, 1));
            dc.DrawLine(tabRect.GetRight(), tabRect.GetTop() + 2, 
                        tabRect.GetRight(), tabRect.GetBottom());
            
            // No bottom border to blend with page
        }
        else // Inactive Tab
        {
            // No background fill (transparent)
            // No border
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(*wxTRANSPARENT_PEN);
            // We still need to draw something if we want the bar background to show, 
            // or just rely on text drawing.
            // dc.DrawRectangle(tabRect); // This would draw with transparent pen/brush over bar bg
            dc.SetTextForeground(FLATUI_BAR_INACTIVE_TEXT_COLOUR); 
        }
        
        dc.DrawText(label, currentXOffsetInOut + FLATUI_BAR_TAB_PADDING, tabYPos + (barEffectiveHeight - labelSize.GetHeight()) / 2); 
        currentXOffsetInOut += tabWidth + FLATUI_BAR_TAB_SPACING; 
    }
}

void FlatUIBar::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    int barEffectiveHeight = GetBarHeight();

    // Mouse events on HomeSpace, FunctionSpace, ProfileSpace, SystemButtons are handled by themselves.
    // This OnMouseDown now only needs to care about clicks on the TABS area.

    int tabRegionStartX = FLATUI_BAR_BAR_PADDING;
    if (m_homeSpace && m_homeSpace->IsShown()) {
        tabRegionStartX = m_homeSpace->GetRect().GetRight() + FLATUI_BAR_ELEMENT_SPACING;
    }
    
    int tabRegionEndX = GetClientSize().GetWidth() - FLATUI_BAR_BAR_PADDING;
    if (m_systemButtons && m_systemButtons->IsShown()) {
        tabRegionEndX = m_systemButtons->GetRect().GetLeft() - FLATUI_BAR_ELEMENT_SPACING;
    }
    // Refine tabRegionEndX based on visible Function/Profile spaces that are to the left of system buttons
    // and to the right of where tabs start.
    if (m_profileSpace && m_profileSpace->IsShown()) {
        int profileLeft = m_profileSpace->GetRect().GetLeft();
        if (profileLeft > tabRegionStartX) { // Profile space is to the right of tabs
            tabRegionEndX = wxMin(tabRegionEndX, profileLeft - FLATUI_BAR_ELEMENT_SPACING);
        }
    }
    if (m_functionSpace && m_functionSpace->IsShown()) {
        int funcLeft = m_functionSpace->GetRect().GetLeft();
        if (funcLeft > tabRegionStartX) { // Function space is to the right of tabs
             tabRegionEndX = wxMin(tabRegionEndX, funcLeft - FLATUI_BAR_ELEMENT_SPACING);
        }
    }


    if (pos.y >= 0 && pos.y < barEffectiveHeight && pos.x >= tabRegionStartX && pos.x < tabRegionEndX) {
        wxClientDC dc(this); 
        int currentXOffset = tabRegionStartX;
    for (size_t i = 0; i < m_pages.size(); ++i)
        {
            if (!m_pages[i]) continue;
            FlatUIPage* page = m_pages[i];
            wxString label = page->GetLabel();
            wxSize labelSize = dc.GetTextExtent(label);
            int tabWidth = labelSize.GetWidth() + FLATUI_BAR_TAB_PADDING * 2;
            wxRect tabRect(currentXOffset, 0, tabWidth, barEffectiveHeight);

            if (tabRect.Contains(pos)) {
                if (m_activePage != i) {
                    SetActivePage(i);
                }
                evt.Skip(false); 
                return;
            }
            currentXOffset += tabWidth + FLATUI_BAR_TAB_SPACING;
            if (currentXOffset >= tabRegionEndX) break; 
        }
    }
    evt.Skip(); 
}

void FlatUIBar::OnSize(wxSizeEvent& evt)
{
    wxSize newSize = GetClientSize();
    LOG_DBG("FlatUIBar::OnSize - New size: (" + std::to_string(newSize.GetWidth()) + ", " + std::to_string(newSize.GetHeight()) + ")", "FlatUIBar");
    
    // Ensure bar height is not less than required minimum
    if (newSize.GetHeight() < GetBarHeight()) {
        LOG_DBG("Warning: FlatUIBar height(" + std::to_string(newSize.GetHeight()) + ") is less than required minimum height(" + std::to_string(GetBarHeight()) + ")", "FlatUIBar");
    }
    
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

void FlatUIBar::SetTabFunctionSpacer(int width, bool drawSeparator, bool dragFlag)
{
    if (!m_tabFunctionSpacer) {
        m_tabFunctionSpacer = new FlatUISpacerControl(this, width);
        m_tabFunctionSpacer->SetCanDragWindow(true);
        m_tabFunctionSpacer->SetDoubleBuffered(true);
    }
    
    if (width > 0) {
        m_tabFunctionSpacer->SetSpacerWidth(width);
        m_tabFunctionSpacer->SetDrawSeparator(drawSeparator);
        m_tabFunctionSpacer->SetShowDragFlag(dragFlag);
        m_tabFunctionSpacer->Show();
        LOG_INF("FlatUIBar: Show TabFunctionSpacer，Width=" + std::to_string(width), "FlatUIBar");
    } else {
        m_tabFunctionSpacer->Hide();
        LOG_INF("FlatUIBar: Hidden TabFunctionSpacer", "FlatUIBar");
    }
    
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::SetFunctionProfileSpacer(int width, bool drawSeparator, bool dragFlag)
{
    if (!m_functionProfileSpacer) {
        m_functionProfileSpacer = new FlatUISpacerControl(this, width);
        m_functionProfileSpacer->SetCanDragWindow(true);
        m_functionProfileSpacer->SetDoubleBuffered(true);
    }
    
    if (width > 0) {
        m_functionProfileSpacer->SetSpacerWidth(width);
        m_functionProfileSpacer->SetDrawSeparator(drawSeparator);
        m_functionProfileSpacer->SetShowDragFlag(dragFlag);
        m_functionProfileSpacer->Show();
        LOG_INF("FlatUIBar: Show FunctionProfileSpacer，Width=" + std::to_string(width), "FlatUIBar");
    } else {
        m_functionProfileSpacer->Hide();
        LOG_INF("FlatUIBar: Hidden FunctionProfileSpacer", "FlatUIBar");
    }
    
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::SetTabFunctionSpacerAutoExpand(bool autoExpand)
{
    if (m_tabFunctionSpacer) {
        m_tabFunctionSpacer->SetAutoExpand(autoExpand);
        
        if (IsShown()) {
            UpdateElementPositionsAndSizes(GetClientSize());
            Refresh();
        }
    }
}

void FlatUIBar::SetFunctionProfileSpacerAutoExpand(bool autoExpand)
{
    if (m_functionProfileSpacer) {
        m_functionProfileSpacer->SetAutoExpand(autoExpand);
        
        if (IsShown()) {
            UpdateElementPositionsAndSizes(GetClientSize());
            Refresh();
        }
    }
}