#include "flatui/FlatUIBar.h"
#include <wx/dcbuffer.h>
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"
#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)

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

void FlatUIBar::SetFunctionSpaceCenterAlign(bool center)
{
    m_functionSpaceCenterAlign = center;
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::SetProfileSpaceRightAlign(bool rightAlign)
{
    m_profileSpaceRightAlign = rightAlign;
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::AddSpaceSeparator(SpacerLocation location, int width, bool drawSeparator, bool canDrag, bool autoExpand)
{
    FlatUISpacerControl** targetSpacer = nullptr;
    wxString logLocation;
    wxString spacerName;

    switch (location) {
    case SPACER_TAB_FUNCTION:
        targetSpacer = &m_tabFunctionSpacer;
        logLocation = "TabFunction";
        spacerName = "TabFunctionSpacer";
        break;
    case SPACER_FUNCTION_PROFILE:
        targetSpacer = &m_functionProfileSpacer;
        logLocation = "FunctionProfile";
        spacerName = "FunctionProfileSpacer";
        break;
    default:
        LOG_ERR("FlatUIBar::AddSpaceSeparator - Invalid location specified", "FlatUIBar");
        return;
    }

    if (!*targetSpacer) {
        *targetSpacer = new FlatUISpacerControl(this, width);
        (*targetSpacer)->SetName(spacerName);
        (*targetSpacer)->SetCanDragWindow(canDrag);
        (*targetSpacer)->SetDoubleBuffered(true);
    }

    if (width > 0) {
        (*targetSpacer)->SetSpacerWidth(width);
        (*targetSpacer)->SetDrawSeparator(drawSeparator);
        (*targetSpacer)->SetShowDragFlag(canDrag);
        (*targetSpacer)->SetAutoExpand(autoExpand);
        (*targetSpacer)->Show();
    }
    else {
        (*targetSpacer)->Hide();
    }

    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::UpdateElementPositionsAndSizes(const wxSize& barClientSz)
{
    if (!m_homeSpace || !m_systemButtons || !m_functionSpace || !m_profileSpace) {
        return; // Components not ready
    }

    // Ensure all controls have proper names
    if (m_homeSpace) m_homeSpace->SetName("HomeSpace");
    if (m_systemButtons) m_systemButtons->SetName("SystemButtons");
    if (m_functionSpace) m_functionSpace->SetName("FunctionSpace");
    if (m_profileSpace) m_profileSpace->SetName("ProfileSpace");
    if (m_tabFunctionSpacer) m_tabFunctionSpacer->SetName("TabFunctionSpacer");
    if (m_functionProfileSpacer) m_functionProfileSpacer->SetName("FunctionProfileSpacer");

    wxClientDC dc(this);
    int barPadding = CFG_INT("BarPadding");
    int elemSpacing = CFG_INT("BarElementSpacing");
    int currentX = barPadding; // Initial currentX before homeSpace
    int barStripHeight = GetBarHeight();
    int innerHeight = barStripHeight - m_barTopMargin - m_barBottomMargin;
    int elementY = m_barTopMargin;

    // Home Space (Leftmost)
    if (m_homeSpace && m_homeSpace->IsShown()) {
            int bW = m_homeSpace->GetButtonWidth();
            m_homeSpace->SetPosition(wxPoint(currentX, elementY));
            m_homeSpace->SetSize(bW, innerHeight);
        m_homeSpace->Show(true);
        currentX += bW + elemSpacing;
    }
    else {
        if (m_homeSpace) m_homeSpace->Show(false);
    }

    // System Buttons (Rightmost) - Calculate position first
    int sysButtonsWidth = 0;
    if (m_systemButtons && m_systemButtons->IsShown()) {
        sysButtonsWidth = m_systemButtons->GetRequiredWidth();
        int ctrlX = barClientSz.GetWidth() - barPadding - sysButtonsWidth;
        m_systemButtons->SetPosition(wxPoint(ctrlX, elementY));
        m_systemButtons->SetSize(sysButtonsWidth, innerHeight);
        m_systemButtons->Show(true);
    }
    else {
        if (m_systemButtons) m_systemButtons->Show(false);
    }

    // Calculate right boundary for flexible elements (excluding system buttons)
    int rightBoundaryForFlexibleElements = barClientSz.GetWidth() - barPadding;
    if (sysButtonsWidth > 0) {
        rightBoundaryForFlexibleElements -= (sysButtonsWidth + elemSpacing);
    }

    // Tabs
    int tabsNeededWidth = CalculateTabsWidth(dc);
    if (tabsNeededWidth > 0) {
        m_tabAreaRect = wxRect(currentX, elementY, tabsNeededWidth, barStripHeight);
        currentX += tabsNeededWidth + elemSpacing;
    }
    else {
        m_tabAreaRect = wxRect();
    }

    // Get visibility and requested widths for function and profile spaces
    int funcRequestedWidth = 0;
    bool funcSpaceIsEffectivelyVisible = m_functionSpace && m_functionSpace->IsShown() && m_functionSpace->GetChildControl();
    if (funcSpaceIsEffectivelyVisible) {
        funcRequestedWidth = m_functionSpace->GetSpaceWidth();
    }
    else {
        if (m_functionSpace) m_functionSpace->Show(false);
    }

    int profileRequestedWidth = 0;
    bool profileSpaceIsEffectivelyVisible = m_profileSpace && m_profileSpace->IsShown() && m_profileSpace->GetChildControl();
    if (profileSpaceIsEffectivelyVisible) {
        profileRequestedWidth = m_profileSpace->GetSpaceWidth();
        }
        else {
        if (m_profileSpace) m_profileSpace->Show(false);
    }

    // Get spacer states
    bool tabFuncSpacerVisible = m_tabFunctionSpacer && m_tabFunctionSpacer->IsShown();
    bool tabFuncSpacerAutoExpand = tabFuncSpacerVisible && m_tabFunctionSpacer->GetAutoExpand();
    bool funcProfileSpacerVisible = m_functionProfileSpacer && m_functionProfileSpacer->IsShown();
    bool funcProfileSpacerAutoExpand = funcProfileSpacerVisible && m_functionProfileSpacer->GetAutoExpand();

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

    // --- Core Layout Logic ---
    if (m_functionSpaceCenterAlign && funcSpaceIsEffectivelyVisible) {
        // Calculate space distribution for centering
        int spaceBeforeFunction = remainingSpace / 2;
        int spaceAfterFunction = remainingSpace - spaceBeforeFunction;

        // Position tabFunctionSpacer (left spacer)
        if (tabFuncSpacerVisible) { 
            int spacerWidth = m_tabFunctionSpacer->GetSpacerWidth();
            if (tabFuncSpacerAutoExpand) {
                spacerWidth = spaceBeforeFunction;
            }
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(spacerWidth, innerHeight);
            m_tabFunctionSpacer->Show(true);
            currentX += spacerWidth;
        }
        else if (spaceBeforeFunction > 0) {
            currentX += spaceBeforeFunction;
                    }

        // Position function space
        if (funcSpaceIsEffectivelyVisible) {
            m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, innerHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
        }

        // Add spacing between function and profile if both are visible
        if (funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                currentX += elemSpacing;
            }

        // Calculate the actual space available for the right spacer
        int rightSpacerAvailableSpace = 0;
        if (profileSpaceIsEffectivelyVisible) {
            // Calculate space between function space and profile space
            rightSpacerAvailableSpace = rightBoundaryForFlexibleElements - currentX - profileRequestedWidth;
            if (funcSpaceIsEffectivelyVisible) {
                rightSpacerAvailableSpace -= elemSpacing; // Account for spacing between function and profile
            }
        }
        rightSpacerAvailableSpace = wxMax(0, rightSpacerAvailableSpace);

        // Position functionProfileSpacer (right spacer)
        if (funcProfileSpacerVisible) {
            int spacerWidth = m_functionProfileSpacer->GetSpacerWidth();
            if (funcProfileSpacerAutoExpand) {
                spacerWidth = rightSpacerAvailableSpace;
            }
            m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            m_functionProfileSpacer->SetSize(spacerWidth, innerHeight);
            m_functionProfileSpacer->Show(true);
            currentX += spacerWidth;
            }
        else if (rightSpacerAvailableSpace > 0) {
            currentX += rightSpacerAvailableSpace;
            }

        // Position profile space at the right (before system buttons)
            if (profileSpaceIsEffectivelyVisible) {
            int profileX = rightBoundaryForFlexibleElements - profileRequestedWidth;
            m_profileSpace->SetPosition(wxPoint(profileX, elementY));
                m_profileSpace->SetSize(profileRequestedWidth, innerHeight);
                m_profileSpace->Show(true);
        }
    }
    else {
        // Sequential layout logic
        // Position tabFunctionSpacer
            if (tabFuncSpacerVisible) {
            int spacerWidth = m_tabFunctionSpacer->GetSpacerWidth();
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
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(spacerWidth, innerHeight);
            m_tabFunctionSpacer->Show(true);
            currentX += spacerWidth;
        }

        // Position function space
        if (funcSpaceIsEffectivelyVisible) {
            if (currentX > barPadding) { // Add spacing if there are elements before
                currentX += elemSpacing;
            }
            m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, innerHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
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
            int spacerWidth = m_functionProfileSpacer->GetSpacerWidth();
            if (funcProfileSpacerAutoExpand) {
                spacerWidth = rightSpacerAvailableSpace;
            }
            if (funcSpaceIsEffectivelyVisible) {
                currentX += elemSpacing;
            }
            m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            m_functionProfileSpacer->SetSize(spacerWidth, innerHeight);
            m_functionProfileSpacer->Show(true);
            currentX += spacerWidth;
        }

        // Position profile space at the right (before system buttons)
        if (profileSpaceIsEffectivelyVisible) {
            int profileX = rightBoundaryForFlexibleElements - profileRequestedWidth;
            m_profileSpace->SetPosition(wxPoint(profileX, elementY));
            m_profileSpace->SetSize(profileRequestedWidth, innerHeight);
            m_profileSpace->Show(true);
        }
    }

    // Page handling (below the bar)
    if (m_activePage < m_pages.size() && m_pages[m_activePage])
    {
        auto* currentPage = m_pages[m_activePage].get();
        currentPage->SetPosition(wxPoint(0, barStripHeight + m_barTopMargin));

        int pageHeight = barClientSz.GetHeight() - barStripHeight - m_barTopMargin;
        if (pageHeight < 0) {
            pageHeight = 0;
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

    Refresh(); // Re-draw the bar itself
}

int FlatUIBar::CalculateTabsWidth(wxDC& dc) const
{
    int tabPadding = CFG_INT("BarTabPadding");
    int tabSpacing = CFG_INT("BarTabSpacing");
    int totalWidth = 0;
    if (m_pages.empty()) return 0;

    for (size_t i = 0; i < m_pages.size(); ++i)
    {
        if (!m_pages[i]) continue;
        auto* page = m_pages[i].get();
        wxString label = page->GetLabel();
        wxSize labelSize = dc.GetTextExtent(label);
        totalWidth += labelSize.GetWidth() + tabPadding * 2;
        if (i < m_pages.size() - 1)
        {
            totalWidth += tabSpacing;
        }
    }
    // Add extra space for the right border of the last tab
    if (!m_pages.empty() && m_tabBorderRight > 0) {
        totalWidth += 1;  // Reserve space for right border
    }

    return totalWidth;
}

void FlatUIBar::ToggleFunctionSpaceVisibility()
{
    if (m_functionSpace) {
        bool visible = m_functionSpace->IsShown();
        bool newVisible = !visible;
        m_functionSpace->Show(newVisible);
        if (m_tabFunctionSpacer) {
            m_tabFunctionSpacer->Show(newVisible);
        }
        if (IsShown()) {
            UpdateElementPositionsAndSizes(GetClientSize());
            Refresh();
        }
    }
}

void FlatUIBar::ToggleProfileSpaceVisibility()
{
    if (m_profileSpace) {
        bool visible = m_profileSpace->IsShown();
        bool newVisible = !visible;
        m_profileSpace->Show(newVisible);
        if (m_functionProfileSpacer) {
            m_functionProfileSpacer->Show(newVisible);
        }
        if (IsShown()) {
            UpdateElementPositionsAndSizes(GetClientSize());
            Refresh();
        }
    }
} 