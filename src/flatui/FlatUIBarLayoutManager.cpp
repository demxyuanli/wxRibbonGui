#include "flatui/FlatUIBarLayoutManager.h"
#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIBarConfig.h"
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"
#include <wx/dcmemory.h>

#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)

FlatUIBarLayoutManager::FlatUIBarLayoutManager(FlatUIBar* bar)
    : m_bar(bar),
      m_layoutValid(false),
      m_layoutDirty(false),
      m_refreshPending(false)
{
    LOG_INF("FlatUIBarLayoutManager initialized", "LayoutManager");
}

void FlatUIBarLayoutManager::UpdateLayout(const wxSize& barClientSize)
{
    if (!m_bar || barClientSize.GetWidth() <= 0 || barClientSize.GetHeight() <= 0) {
        LOG_ERR("Invalid parameters for UpdateLayout", "LayoutManager");
        return;
    }

    LOG_INF("Updating layout for size (" + std::to_string(barClientSize.GetWidth()) + 
           "," + std::to_string(barClientSize.GetHeight()) + ")", "LayoutManager");

    // Cache the new size
    m_lastBarSize = barClientSize;
    m_layoutDirty = false;
    
    // Get all child components from the bar
    FlatUIHomeSpace* homeSpace = m_bar->GetHomeSpace();
    FlatUISystemButtons* systemButtons = m_bar->GetSystemButtons();
    FlatUIFunctionSpace* functionSpace = m_bar->GetFunctionSpace();
    FlatUIProfileSpace* profileSpace = m_bar->GetProfileSpace();
    FlatUISpacerControl* tabFunctionSpacer = m_bar->GetTabFunctionSpacer();
    FlatUISpacerControl* functionProfileSpacer = m_bar->GetFunctionProfileSpacer();
    FlatUIFixPanel* fixPanel = m_bar->GetFixPanel();
    
    // Check if basic components are ready
    if (!homeSpace) {
        LOG_ERR("Core components not ready for layout", "LayoutManager");
        return;
    }
    
    // Create a temporary DC for calculations
    wxMemoryDC dc;
    wxBitmap tempBitmap(1, 1);
    dc.SelectObject(tempBitmap);
    
    // Basic layout parameters
    int barPadding = GetBarPadding();
    int elemSpacing = GetElementSpacing();
    int currentX = barPadding;
    int barStripHeight = m_bar->GetBarHeight();
    int barTopMargin = m_bar->GetBarTopMargin();
    int barBottomMargin = m_bar->GetBarBottomMargin();
    int innerHeight = barStripHeight - barTopMargin - barBottomMargin;
    int elementY = barTopMargin;
    
    // Debug: Log bar measurements
    LOG_INF("Bar measurements: barStripHeight=" + std::to_string(barStripHeight) + 
           ", barTopMargin=" + std::to_string(barTopMargin) + 
           ", barBottomMargin=" + std::to_string(barBottomMargin) + 
           ", innerHeight=" + std::to_string(innerHeight), "LayoutManager");

    // Home Space (Leftmost)
    if (homeSpace && homeSpace->IsShown()) {
        int bW = homeSpace->GetButtonWidth();
        homeSpace->SetPosition(wxPoint(currentX, elementY));
        homeSpace->SetSize(bW, innerHeight);
        homeSpace->Show(true);
        currentX += bW + elemSpacing;
        LOG_INF("Positioned HomeSpace at (" + std::to_string(currentX - bW - elemSpacing) + 
               "," + std::to_string(elementY) + ") size (" + std::to_string(bW) + 
               "," + std::to_string(innerHeight) + ")", "LayoutManager");
    }
    else {
        if (homeSpace) homeSpace->Show(false);
    }

    // System Buttons (Rightmost) - Calculate position first
    int sysButtonsWidth = 0;
    if (systemButtons && systemButtons->IsShown()) {
        sysButtonsWidth = systemButtons->GetRequiredWidth();
        int ctrlX = barClientSize.GetWidth() - barPadding - sysButtonsWidth;
        systemButtons->SetPosition(wxPoint(ctrlX, elementY));
        systemButtons->SetSize(sysButtonsWidth, innerHeight);
        systemButtons->Show(true);
        LOG_INF("Positioned SystemButtons at (" + std::to_string(ctrlX) + 
               "," + std::to_string(elementY) + ")", "LayoutManager");
    }

    // Calculate right boundary for flexible elements (excluding system buttons)
    int rightBoundaryForFlexibleElements = barClientSize.GetWidth() - barPadding;
    if (sysButtonsWidth > 0) {
        rightBoundaryForFlexibleElements -= (sysButtonsWidth + elemSpacing);
    }

    // Tabs
    int tabsNeededWidth = CalculateTabsWidth(dc);
    wxRect tabAreaRect;
    if (tabsNeededWidth > 0) {
        tabAreaRect = wxRect(currentX, elementY, tabsNeededWidth, barStripHeight);
        currentX += tabsNeededWidth + elemSpacing;
        LOG_INF("Tab area calculated: width=" + std::to_string(tabsNeededWidth), "LayoutManager");
    }
    else {
        tabAreaRect = wxRect();
    }
    
    // Store tab area for painting
    m_bar->SetTabAreaRect(tabAreaRect);

    // Function and Profile spaces positioning with proper layout logic
    
    // Get visibility and requested widths for function and profile spaces
    int funcRequestedWidth = 0;
    bool funcSpaceIsEffectivelyVisible = functionSpace && functionSpace->IsShown() && functionSpace->GetChildControl();
    if (funcSpaceIsEffectivelyVisible) {
        funcRequestedWidth = functionSpace->GetSpaceWidth();
    }
    else {
        if (functionSpace) functionSpace->Show(false);
    }

    int profileRequestedWidth = 0;
    bool profileSpaceIsEffectivelyVisible = profileSpace && profileSpace->IsShown() && profileSpace->GetChildControl();
    if (profileSpaceIsEffectivelyVisible) {
        profileRequestedWidth = profileSpace->GetSpaceWidth();
    }
    else {
        if (profileSpace) profileSpace->Show(false);
    }

    // Get spacer states
    bool tabFuncSpacerVisible = tabFunctionSpacer && tabFunctionSpacer->IsShown();
    bool tabFuncSpacerAutoExpand = tabFuncSpacerVisible && tabFunctionSpacer->GetAutoExpand();
    bool funcProfileSpacerVisible = functionProfileSpacer && functionProfileSpacer->IsShown();
    bool funcProfileSpacerAutoExpand = funcProfileSpacerVisible && functionProfileSpacer->GetAutoExpand();

    // Calculate total width needed for all elements
    int totalFixedWidth = 0;
    if (funcSpaceIsEffectivelyVisible) {
        totalFixedWidth += funcRequestedWidth;
    }
    if (profileSpaceIsEffectivelyVisible) {
        totalFixedWidth += profileRequestedWidth;
        if (funcSpaceIsEffectivelyVisible) {
            totalFixedWidth += elemSpacing; // Spacing between function and profile
        }
    }

    // Calculate available space for flexible elements
    int availableWidth = rightBoundaryForFlexibleElements - currentX;
    availableWidth = wxMax(0, availableWidth);

    // Calculate remaining space to distribute
    int remainingSpace = availableWidth - totalFixedWidth;
    remainingSpace = wxMax(0, remainingSpace);

    // Layout Logic - Check if function space should be centered
    if (m_bar->GetFunctionSpaceCenterAlign() && funcSpaceIsEffectivelyVisible) {
        // Calculate space distribution for centering
        int spaceBeforeFunction = remainingSpace / 2;
        int spaceAfterFunction = remainingSpace - spaceBeforeFunction;

        // Position tabFunctionSpacer (left spacer)
        if (tabFuncSpacerVisible) { 
            int spacerWidth = tabFunctionSpacer->GetSpacerWidth();
            if (tabFuncSpacerAutoExpand) {
                spacerWidth = spaceBeforeFunction;
            }
            tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            tabFunctionSpacer->SetSize(spacerWidth, innerHeight);
            tabFunctionSpacer->Show(true);
            currentX += spacerWidth;
            LOG_INF("Positioned TabFunctionSpacer for centering: width=" + std::to_string(spacerWidth), "LayoutManager");
        }
        else if (spaceBeforeFunction > 0) {
            currentX += spaceBeforeFunction;
        }

        // Position function space in center
        if (funcSpaceIsEffectivelyVisible) {
            functionSpace->SetPosition(wxPoint(currentX, elementY));
            functionSpace->SetSize(funcRequestedWidth, innerHeight);
            functionSpace->Show(true);
            currentX += funcRequestedWidth;
            LOG_INF("Positioned FunctionSpace (centered) at (" + std::to_string(currentX - funcRequestedWidth) + 
                   "," + std::to_string(elementY) + ")", "LayoutManager");
        }

        // Add spacing between function and profile if both are visible
        if (funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            currentX += elemSpacing;
        }

        // Calculate the actual space available for the right spacer
        int rightSpacerAvailableSpace = 0;
        if (profileSpaceIsEffectivelyVisible) {
            rightSpacerAvailableSpace = rightBoundaryForFlexibleElements - currentX - profileRequestedWidth;
        }
        rightSpacerAvailableSpace = wxMax(0, rightSpacerAvailableSpace);

        // Position functionProfileSpacer (right spacer)
        if (funcProfileSpacerVisible) {
            int spacerWidth = functionProfileSpacer->GetSpacerWidth();
            if (funcProfileSpacerAutoExpand) {
                spacerWidth = rightSpacerAvailableSpace;
            }
            functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            functionProfileSpacer->SetSize(spacerWidth, innerHeight);
            functionProfileSpacer->Show(true);
            currentX += spacerWidth;
            LOG_INF("Positioned FunctionProfileSpacer: width=" + std::to_string(spacerWidth), "LayoutManager");
        }
        else if (rightSpacerAvailableSpace > 0) {
            currentX += rightSpacerAvailableSpace;
        }

        // Position profile space at the right (before system buttons)
        if (profileSpaceIsEffectivelyVisible) {
            int profileX = rightBoundaryForFlexibleElements - profileRequestedWidth;
            profileSpace->SetPosition(wxPoint(profileX, elementY));
            profileSpace->SetSize(profileRequestedWidth, innerHeight);
            profileSpace->Show(true);
            LOG_INF("Positioned ProfileSpace at right: (" + std::to_string(profileX) + 
                   "," + std::to_string(elementY) + ")", "LayoutManager");
        }
    }
    else {
        // Sequential layout logic (no centering)
        // Position tabFunctionSpacer
        if (tabFuncSpacerVisible) {
            int spacerWidth = tabFunctionSpacer->GetSpacerWidth();
            if (tabFuncSpacerAutoExpand) {
                // Calculate available space for both spacers
                int availableSpaceForSpacers = rightBoundaryForFlexibleElements - currentX;
                if (profileSpaceIsEffectivelyVisible) {
                    availableSpaceForSpacers -= (profileRequestedWidth + elemSpacing);
                }
                if (funcSpaceIsEffectivelyVisible) {
                    availableSpaceForSpacers -= (funcRequestedWidth + elemSpacing);
                }
                availableSpaceForSpacers = wxMax(0, availableSpaceForSpacers);
                spacerWidth = availableSpaceForSpacers / 2; // Use half of available space
            }
            tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            tabFunctionSpacer->SetSize(spacerWidth, innerHeight);
            tabFunctionSpacer->Show(true);
            currentX += spacerWidth;
        }

        // Position function space
        if (funcSpaceIsEffectivelyVisible) {
            if (currentX > barPadding) { // Add spacing if there are elements before
                currentX += elemSpacing;
            }
            functionSpace->SetPosition(wxPoint(currentX, elementY));
            functionSpace->SetSize(funcRequestedWidth, innerHeight);
            functionSpace->Show(true);
            currentX += funcRequestedWidth;
            LOG_INF("Positioned FunctionSpace (sequential) at (" + std::to_string(currentX - funcRequestedWidth) + 
                   "," + std::to_string(elementY) + ")", "LayoutManager");
        }

        // Calculate available space for right spacer
        int rightSpacerAvailableSpace = 0;
        if (profileSpaceIsEffectivelyVisible) {
            rightSpacerAvailableSpace = rightBoundaryForFlexibleElements - currentX - profileRequestedWidth;
            if (funcSpaceIsEffectivelyVisible) {
                rightSpacerAvailableSpace -= elemSpacing; // Account for spacing between function and profile
            }
        }
        rightSpacerAvailableSpace = wxMax(0, rightSpacerAvailableSpace);

        // Position functionProfileSpacer
        if (funcProfileSpacerVisible) {
            int spacerWidth = functionProfileSpacer->GetSpacerWidth();
            if (funcProfileSpacerAutoExpand) {
                spacerWidth = rightSpacerAvailableSpace;
            }
            if (funcSpaceIsEffectivelyVisible) {
                currentX += elemSpacing;
            }
            functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            functionProfileSpacer->SetSize(spacerWidth, innerHeight);
            functionProfileSpacer->Show(true);
            currentX += spacerWidth;
        }

        // Position profile space at the right (before system buttons)
        if (profileSpaceIsEffectivelyVisible) {
            int profileX = rightBoundaryForFlexibleElements - profileRequestedWidth;
            profileSpace->SetPosition(wxPoint(profileX, elementY));
            profileSpace->SetSize(profileRequestedWidth, innerHeight);
            profileSpace->Show(true);
            LOG_INF("Positioned ProfileSpace at right: (" + std::to_string(profileX) + 
                   "," + std::to_string(elementY) + ")", "LayoutManager");
        }
    }

    // FixPanel handling based on pin state
    if (fixPanel && m_bar->GetStateManager()) {
        bool shouldShowFixPanel = m_bar->GetStateManager()->IsPinned();
        
        if (shouldShowFixPanel) {
            // Calculate the correct Y position for FixPanel
            // Use the bar strip height + margins for proper positioning 
            int totalBarHeight = barStripHeight + barBottomMargin;
            int FIXED_PANEL_Y = totalBarHeight;
            
            // Safety check: Ensure we have a minimum Y position
            if (FIXED_PANEL_Y <= 0) {
                FIXED_PANEL_Y = 31; // Standard fallback based on common bar height
                LOG_INF("WARNING: Calculated bar height is " + std::to_string(totalBarHeight) + 
                       ", using fallback Y position " + std::to_string(FIXED_PANEL_Y), "LayoutManager");
            }
            
            // Calculate proposed FixPanel height
            int fixPanelHeight = barClientSize.GetHeight() - FIXED_PANEL_Y;
            
            // Minimum viable height check to prevent invisible or overlapping panels
            const int MIN_FIXPANEL_HEIGHT = 60; // Must be at least 20px to be useful
            if (fixPanelHeight < MIN_FIXPANEL_HEIGHT) {
                LOG_WRN("FixPanel height too small (" + std::to_string(fixPanelHeight) + 
                       "), deferring positioning to prevent barspace overlap", "LayoutManager");
                
                // Schedule a delayed layout update to retry when window size is proper
                m_bar->CallAfter([this]() {
                    if (m_bar && m_bar->GetSize().GetHeight() > 50) { // Minimum reasonable window height
                        UpdateLayout(m_bar->GetSize());
                    }
                });
                return; // Don't position with invalid height
            }
            
            // Debug logging
            LOG_INF("FixPanel positioning: barStripHeight=" + std::to_string(barStripHeight) + 
                   ", barTopMargin=" + std::to_string(barTopMargin) + 
                   ", barBottomMargin=" + std::to_string(barBottomMargin) + 
                   ", totalBarHeight=" + std::to_string(totalBarHeight) + 
                   ", FIXED_PANEL_Y=" + std::to_string(FIXED_PANEL_Y), "LayoutManager");
            
            fixPanel->SetPosition(wxPoint(0, FIXED_PANEL_Y));
            fixPanel->SetSize(barClientSize.GetWidth(), fixPanelHeight);

            LOG_INF("Positioned FixPanel at (0, " + std::to_string(FIXED_PANEL_Y) + 
                   ") with size (" + std::to_string(barClientSize.GetWidth()) + 
                   ", " + std::to_string(fixPanelHeight) + ")", "LayoutManager");

            if (!fixPanel->IsShown()) {
                fixPanel->Show();
                LOG_INF("Showed FixPanel", "LayoutManager");
            }

            // Set active page in FixPanel
            if (m_bar->GetPageCount() > 0) {
                size_t activePage = m_bar->GetActivePage();
                fixPanel->SetActivePage(activePage);
            }
        } else {
            // Hide FixPanel in unpinned state
            if (fixPanel->IsShown()) {
                fixPanel->Hide();
                LOG_INF("Hidden FixPanel", "LayoutManager");
            }
        }
    }

    LOG_INF("Layout update completed", "LayoutManager");
    m_layoutValid = true;
    
    // Only use deferred refresh if not called during state transition
    // During state transitions, the calling code manages refresh timing
    if (m_bar && !m_bar->IsFrozen()) {
        // Only refresh if the window is not frozen (indicating no ongoing state transition)
        DeferredRefresh();
    } else {
        LOG_INF("Skipping DeferredRefresh during state transition (window frozen)", "LayoutManager");
    }
}

void FlatUIBarLayoutManager::UpdateLayoutIfNeeded(const wxSize& barClientSize)
{
    if (m_layoutDirty || barClientSize != m_lastBarSize || !m_layoutValid) {
        UpdateLayout(barClientSize);
    }
}

void FlatUIBarLayoutManager::MarkLayoutDirty()
{
    m_layoutDirty = true;
}

void FlatUIBarLayoutManager::ForceRefresh()
{
    if (m_bar && m_bar->IsShown()) {
        m_bar->Refresh();
        m_bar->Update();
    }
}

void FlatUIBarLayoutManager::DeferredRefresh()
{
    if (!m_refreshPending && m_bar && m_bar->IsShown()) {
        m_refreshPending = true;
        m_bar->CallAfter([this]() {
            if (m_refreshPending && m_bar && m_bar->IsShown()) {
                m_bar->Refresh();
                m_refreshPending = false;
            }
        });
    }
}

int FlatUIBarLayoutManager::CalculateTabsWidth(wxDC& dc) const
{
    if (!m_bar) return 0;
    
    int tabPadding = CFG_INT("BarTabPadding");
    int tabSpacing = CFG_INT("BarTabSpacing");
    int totalWidth = 0;
    
    size_t pageCount = m_bar->GetPageCount();
    if (pageCount == 0) return 0;

    for (size_t i = 0; i < pageCount; ++i) {
        FlatUIPage* page = m_bar->GetPage(i);
        if (!page) continue;
        
        wxString label = page->GetLabel();
        wxSize labelSize = dc.GetTextExtent(label);
        totalWidth += labelSize.GetWidth() + tabPadding * 2;
        
        if (i < pageCount - 1) {
            totalWidth += tabSpacing;
        }
    }
    
    return totalWidth;
}

wxRect FlatUIBarLayoutManager::CalculateTabAreaRect(int currentX, int elementY, int tabsWidth, int barHeight) const
{
    if (tabsWidth > 0) {
        return wxRect(currentX, elementY, tabsWidth, barHeight);
    }
    return wxRect();
}

void FlatUIBarLayoutManager::UpdateElementVisibility()
{
    // Placeholder - will be implemented as we refactor more
    LOG_INF("UpdateElementVisibility called", "LayoutManager");
}

void FlatUIBarLayoutManager::ShowElement(wxWindow* element, bool show)
{
    if (element && element->IsShown() != show) {
        element->Show(show);
        LOG_INF("Element visibility changed: " + element->GetName().ToStdString() + 
               " -> " + (show ? "shown" : "hidden"), "LayoutManager");
    }
}

bool FlatUIBarLayoutManager::ValidateLayout() const
{
    return m_layoutValid && m_bar != nullptr;
}

void FlatUIBarLayoutManager::LogLayoutInfo(const wxString& context) const
{
    LOG_INF("Layout Info [" + context.ToStdString() + 
           "]: Valid=" + (m_layoutValid ? "true" : "false") + 
           ", Size=(" + std::to_string(m_lastBarSize.GetWidth()) + 
           "," + std::to_string(m_lastBarSize.GetHeight()) + ")", "LayoutManager");
}

int FlatUIBarLayoutManager::GetElementSpacing() const
{
    return FlatUIBarConfig::ELEMENT_SPACING;
}

int FlatUIBarLayoutManager::GetBarPadding() const
{
    return FlatUIBarConfig::BAR_PADDING;
}

bool FlatUIBarLayoutManager::ShouldShowElement(wxWindow* element) const
{
    return element != nullptr && element->IsShown();
} 