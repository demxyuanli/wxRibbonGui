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

// Height of the entire FlatUIBar strip

// Static method implementation
int FlatUIBar::GetBarHeight() 
{
    return FLATUI_BAR_HEIGHT;
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
    // Create child component controls
    m_homeSpace = new FlatUIHomeSpace(this, wxID_ANY);
    m_functionSpace = new FlatUIFunctionSpace(this, wxID_ANY);
    m_profileSpace = new FlatUIProfileSpace(this, wxID_ANY);
    m_systemButtons = new FlatUISystemButtons(this, wxID_ANY);
    
    m_tabFunctionSpacer = new FlatUISpacerControl(this, 0);
    m_functionProfileSpacer = new FlatUISpacerControl(this, 0);
    m_tabFunctionSpacer->Hide();
    m_functionProfileSpacer->Hide();

    // m_pages is default constructed (empty wxVector)

    FlatUIEventManager::getInstance().bindBarEvents(this);
    
    FlatUIEventManager::getInstance().bindHomeSpaceEvents(m_homeSpace);
    FlatUIEventManager::getInstance().bindSystemButtonsEvents(m_systemButtons);
    FlatUIEventManager::getInstance().bindFunctionSpaceEvents(m_functionSpace);
    FlatUIEventManager::getInstance().bindProfileSpaceEvents(m_profileSpace);

    SetBackgroundStyle(wxBG_STYLE_PAINT);
    
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
}

FlatUIBar::~FlatUIBar()
{
    // Child components (m_homeSpace, etc.) are wxWindows and will be deleted by wxWidgets 
    // when this FlatUIBar (their parent) is destroyed.
    for (auto page : m_pages)
        delete page; // Content pages are explicitly managed
}

// --- Configuration Method Implementations ---
void FlatUIBar::SetHomeButtonMenu(wxMenu* menu)
{
    if (m_homeSpace) m_homeSpace->SetMenu(menu);
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
    } 
    
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
        
        wxLogDebug("Added page '%s', total pages: %zu, active page: %zu", 
                  page->GetLabel(), m_pages.size(), m_activePage);
    }
}

void FlatUIBar::SetActivePage(size_t index)
{
    if (index < m_pages.size() && index != m_activePage)
    {
        if (m_activePage < m_pages.size() && m_pages[m_activePage])
        {
            m_pages[m_activePage]->Hide();
        }
        m_activePage = index;
        if (m_pages[m_activePage])
        {
            FlatUIPage* page = m_pages[m_activePage];
            
            wxSize barClientSize = GetClientSize();
            int barStripHeight = GetBarHeight();
            page->SetPosition(wxPoint(0, barStripHeight));
            
            int pageHeight = barClientSize.GetHeight() - barStripHeight;
            if (pageHeight < 0) {
                pageHeight = 0;
            }
            
            page->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));
            
            page->Show();
            
            page->Layout();
            
            wxVector<FlatUIPanel*>& panels = page->GetPanels();
            for (auto panel : panels) {
                if (panel) {
                    panel->SetMinSize(wxSize(100, 100));
                    panel->Layout();
                    panel->Refresh();
                }
            }
            
            page->Refresh();
        }
        
        if (IsShown()) Refresh(); 
    }
    else if (index < m_pages.size() && index == m_activePage)
    { 
        if (m_pages[m_activePage] && !m_pages[m_activePage]->IsShown()) 
        {
            FlatUIPage* page = m_pages[m_activePage];
            
            page->Show();
            
            wxSize barClientSize = GetClientSize();
            int barStripHeight = GetBarHeight();
            int pageHeight = barClientSize.GetHeight() - barStripHeight;
            if (pageHeight < 0) pageHeight = 0;
            
            page->SetSize(wxSize(barClientSize.GetWidth(), pageHeight));
            
            page->Layout();
            
            page->Refresh();
        }
        
        if (IsShown()) Refresh();
    }
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
        totalWidth += labelSize.GetWidth() + TAB_PADDING * 2; 
        if (i < m_pages.size() - 1)
        {
            totalWidth += TAB_SPACING; 
        }
    }
    return totalWidth;
}

void FlatUIBar::UpdateElementPositionsAndSizes(const wxSize& barClientSz)
{
    if (!m_homeSpace || !m_systemButtons || !m_functionSpace || !m_profileSpace) {
        wxLogDebug("FlatUIBar::UpdateElementPositionsAndSizes - one or more child components are null.");
        return; // Components not ready
    }

    wxClientDC dc(this); 
    int currentX = BAR_PADDING;
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
        currentX += homeActualWidth + ELEMENT_SPACING;
    } else {
        m_homeSpace->Show(false);
    }

    int sysButtonsWidth = 0;
    if (m_systemButtons && m_systemButtons->IsShown()) {
        sysButtonsWidth = m_systemButtons->GetMyRequiredWidth();
    }

    int rightBoundary = barClientSz.GetWidth() - BAR_PADDING;
    if (sysButtonsWidth > 0) {
        rightBoundary -= (sysButtonsWidth + ELEMENT_SPACING);
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
            currentX += tabsNeededWidth + ELEMENT_SPACING;
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
                    reservedWidthAfterTabFunc += ELEMENT_SPACING;
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
                    currentX += ELEMENT_SPACING;
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
                currentX += ELEMENT_SPACING;
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
                    reservedWidth += ELEMENT_SPACING;
                }
            }
            
            if (profileSpaceIsEffectivelyVisible) {
                reservedWidth += profileRequestedWidth;
                if (funcSpaceIsEffectivelyVisible) {
                    if (funcProfileSpacerVisible) {
                        reservedWidth += m_functionProfileSpacer->GetSpacerWidth();
                    } else {
                        reservedWidth += ELEMENT_SPACING;
                    }
                } else {
                    if (tabFuncSpacerVisible) {
                        reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
                    } else {
                        reservedWidth += ELEMENT_SPACING;
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
                currentX += ELEMENT_SPACING;
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
                currentX += ELEMENT_SPACING;
            } else if (!funcSpaceIsEffectivelyVisible && tabFuncSpacerVisible && profileSpaceIsEffectivelyVisible) {
                m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
                m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), barStripHeight);
                currentX += m_tabFunctionSpacer->GetSpacerWidth();
            } else if (!funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                currentX += ELEMENT_SPACING;
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
                reservedWidth += ELEMENT_SPACING;
            }
        }
        
        if (profileSpaceIsEffectivelyVisible) {
            reservedWidth += profileRequestedWidth;
            if (funcSpaceIsEffectivelyVisible) {
                if (funcProfileSpacerVisible) {
                    reservedWidth += m_functionProfileSpacer->GetSpacerWidth();
                } else {
                    reservedWidth += ELEMENT_SPACING;
                }
            } else {
                if (tabFuncSpacerVisible) {
                    reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
                } else {
                    reservedWidth += ELEMENT_SPACING;
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
            currentX += ELEMENT_SPACING;
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
            currentX += ELEMENT_SPACING;
        } else if (!funcSpaceIsEffectivelyVisible && tabFuncSpacerVisible && profileSpaceIsEffectivelyVisible) {
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), barStripHeight);
            currentX += m_tabFunctionSpacer->GetSpacerWidth();
        } else if (!funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            currentX += ELEMENT_SPACING;
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
        m_systemButtons->SetPosition(wxPoint(barClientSz.GetWidth() - BAR_PADDING - sysButtonsWidth, elementY));
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
            wxLogDebug("Warning: Page height calculated as negative, adjusted to 0.");
        }
        currentPage->SetSize(barClientSz.GetWidth(), pageHeight);
        
        if (!currentPage->IsShown()) {
            currentPage->Show();
        }
        
        currentPage->Layout();
        currentPage->Refresh();
    }
    for (size_t i = 0; i < m_pages.size(); ++i) {
        if (i != m_activePage && m_pages[i] && m_pages[i]->IsShown()) {
            m_pages[i]->Hide();
        }
    }

    Refresh();
}

void FlatUIBar::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize clientSize = GetClientSize();


    dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_MENUBAR));
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.Clear();
    dc.DrawRectangle(0, 0, clientSize.GetWidth(), clientSize.GetHeight());

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

    for (size_t i = 0; i < m_pages.size(); ++i)
    {
        if (!m_pages[i]) continue;

        FlatUIPage* page = m_pages[i];
        wxString label = page->GetLabel();
        wxSize labelSize = dc.GetTextExtent(label);
        int tabWidth = labelSize.GetWidth() + TAB_PADDING * 2;

        // Check if this tab can fit in the remaining available width
        if ((currentXOffsetInOut + tabWidth) > (initialXOffset + availableTotalWidth) && i > 0) { 
            break; 
        }
        if (tabWidth > availableTotalWidth && i==0 && m_pages.size() > 1) {
            // Handle first tab being too wide
        }
        if ((currentXOffsetInOut - initialXOffset + tabWidth) > availableTotalWidth) {
            if (i > 0) break;
        }

        wxRect tabRect(currentXOffsetInOut, tabYPos, tabWidth, barEffectiveHeight -1); 

        if (i == m_activePage)
        {
            dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT)); 
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT));
        }
        else
        {
            dc.SetBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
            dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT));
        }
        dc.SetPen(*wxGREY_PEN); 
        dc.DrawRectangle(tabRect);
        dc.DrawText(label, currentXOffsetInOut + TAB_PADDING, tabYPos + (barEffectiveHeight - labelSize.GetHeight()) / 2);
        currentXOffsetInOut += tabWidth + TAB_SPACING;
    }
}

void FlatUIBar::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    int barEffectiveHeight = GetBarHeight();

    // Mouse events on HomeSpace, FunctionSpace, ProfileSpace, SystemButtons are handled by themselves.
    // This OnMouseDown now only needs to care about clicks on the TABS area.

    int tabRegionStartX = BAR_PADDING;
    if (m_homeSpace && m_homeSpace->IsShown()) {
        tabRegionStartX = m_homeSpace->GetRect().GetRight() + ELEMENT_SPACING;
    }
    
    int tabRegionEndX = GetClientSize().GetWidth() - BAR_PADDING;
    if (m_systemButtons && m_systemButtons->IsShown()) {
        tabRegionEndX = m_systemButtons->GetRect().GetLeft() - ELEMENT_SPACING;
    }
    // Refine tabRegionEndX based on visible Function/Profile spaces that are to the left of system buttons
    // and to the right of where tabs start.
    if (m_profileSpace && m_profileSpace->IsShown()) {
        int profileLeft = m_profileSpace->GetRect().GetLeft();
        if (profileLeft > tabRegionStartX) { // Profile space is to the right of tabs
            tabRegionEndX = wxMin(tabRegionEndX, profileLeft - ELEMENT_SPACING);
        }
    }
    if (m_functionSpace && m_functionSpace->IsShown()) {
        int funcLeft = m_functionSpace->GetRect().GetLeft();
        if (funcLeft > tabRegionStartX) { // Function space is to the right of tabs
             tabRegionEndX = wxMin(tabRegionEndX, funcLeft - ELEMENT_SPACING);
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
            int tabWidth = labelSize.GetWidth() + TAB_PADDING * 2;
            wxRect tabRect(currentXOffset, 0, tabWidth, barEffectiveHeight);

            if (tabRect.Contains(pos)) {
                if (m_activePage != i) {
                    SetActivePage(i);
                }
                evt.Skip(false); 
                return;
            }
            currentXOffset += tabWidth + TAB_SPACING;
            if (currentXOffset >= tabRegionEndX) break; 
        }
    }
    evt.Skip(); 
}

void FlatUIBar::OnSize(wxSizeEvent& evt)
{
    wxSize newSize = GetClientSize();
    wxLogDebug("FlatUIBar::OnSize - New size: (%d, %d)", newSize.GetWidth(), newSize.GetHeight());
    
    // Ensure bar height is not less than required minimum
    if (newSize.GetHeight() < GetBarHeight()) {
        wxLogDebug("Warning: FlatUIBar height(%d) is less than required minimum height(%d)", 
                   newSize.GetHeight(), GetBarHeight());
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

void FlatUIBar::SetTabFunctionSpacer(int width, bool drawSeparator)
{
    if (!m_tabFunctionSpacer) {
        m_tabFunctionSpacer = new FlatUISpacerControl(this, width);
    }
    
    if (width > 0) {
        m_tabFunctionSpacer->SetSpacerWidth(width);
        m_tabFunctionSpacer->SetDrawSeparator(drawSeparator);
        m_tabFunctionSpacer->Show();
    } else {
        m_tabFunctionSpacer->Hide();
    }
    
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::SetFunctionProfileSpacer(int width, bool drawSeparator)
{
    if (!m_functionProfileSpacer) {
        m_functionProfileSpacer = new FlatUISpacerControl(this, width);
    }
    
    if (width > 0) {
        m_functionProfileSpacer->SetSpacerWidth(width);
        m_functionProfileSpacer->SetDrawSeparator(drawSeparator);
        m_functionProfileSpacer->Show();
    } else {
        m_functionProfileSpacer->Hide();
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