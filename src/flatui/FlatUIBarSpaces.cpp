#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIConstants.h"
#include <wx/dcbuffer.h>
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"
#define CFG_COLOUR(key, def) ConstantsConfig::getInstance().getColourValue(key, def)
#define CFG_INT(key, def)    ConstantsConfig::getInstance().getIntValue(key, def)

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

    switch (location) {
    case SPACER_TAB_FUNCTION:
        targetSpacer = &m_tabFunctionSpacer;
        logLocation = "TabFunction";
        break;
    case SPACER_FUNCTION_PROFILE:
        targetSpacer = &m_functionProfileSpacer;
        logLocation = "FunctionProfile";
        break;
    default:
        LOG_ERR("FlatUIBar::AddSpaceSeparator - Invalid location specified", "FlatUIBar");
        return;
    }

    if (!*targetSpacer) {
        *targetSpacer = new FlatUISpacerControl(this, width);
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

    wxClientDC dc(this);
    int barPadding = CFG_INT("BarPadding", FLATUI_BAR_PADDING);
    int elemSpacing = CFG_INT("BarElementSpacing", FLATUI_BAR_ELEMENT_SPACING);
    int currentX = barPadding;
    int barStripHeight = GetBarHeight() ;
    int innerHeight = barStripHeight - m_barTopMargin - m_barBottomMargin;
    int elementY = m_barTopMargin;  // Use top margin instead of 0

    if (m_homeSpace) {
        if (m_homeSpace->IsShown()) {
            int bW = m_homeSpace->GetButtonWidth();
            int totalW = bW;
            int h = innerHeight;
            if (h < 0) h = 0;
            m_homeSpace->SetPosition(wxPoint(currentX, elementY));
            m_homeSpace->SetSize(bW, innerHeight);
        m_homeSpace->Show(true);
            currentX += totalW + elemSpacing;
        } else {
            m_homeSpace->Show(false);
    }
    }

    int sysButtonsWidth = 0;
    if (m_systemButtons && m_systemButtons->IsShown()) {
        sysButtonsWidth = m_systemButtons->GetRequiredWidth();
    }

    int rightBoundary = barClientSz.GetWidth() - barPadding;
    if (sysButtonsWidth > 0) {
        rightBoundary -= (sysButtonsWidth + elemSpacing);
    }
    else {
        m_systemButtons->Show(false);
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
            currentX += tabsNeededWidth + elemSpacing;
        }
        else {
            m_tabAreaRect = wxRect();
        }

        if (tabFuncSpacerAutoExpand && funcSpaceIsEffectivelyVisible) {
            int reservedWidthAfterTabFunc = 0;

            reservedWidthAfterTabFunc += funcRequestedWidth;

            if (profileSpaceIsEffectivelyVisible) {
                reservedWidthAfterTabFunc += profileRequestedWidth;

                if (funcProfileSpacerVisible && !funcProfileSpacerAutoExpand) {
                    reservedWidthAfterTabFunc += m_functionProfileSpacer->GetSpacerWidth();
                }
                else if (!funcProfileSpacerVisible) {
                    reservedWidthAfterTabFunc += elemSpacing;
                }
            }

            int autoSpacerWidth = rightBoundary - currentX - reservedWidthAfterTabFunc;
            autoSpacerWidth = wxMax(m_tabFunctionSpacer->GetSpacerWidth(), autoSpacerWidth);

            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(autoSpacerWidth, innerHeight);
            currentX += autoSpacerWidth;

            m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, innerHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
            if (profileSpaceIsEffectivelyVisible) {
                if (funcProfileSpacerVisible) {
                    if (funcProfileSpacerAutoExpand) {
                        int remainingSpace = rightBoundary - currentX - profileRequestedWidth;
                        remainingSpace = wxMax(m_functionProfileSpacer->GetSpacerWidth(), remainingSpace);

                        m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
                        m_functionProfileSpacer->SetSize(remainingSpace, innerHeight);
                        currentX += remainingSpace;
                    }
                    else {
                        m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
                        m_functionProfileSpacer->SetSize(m_functionProfileSpacer->GetSpacerWidth(), innerHeight);
                        currentX += m_functionProfileSpacer->GetSpacerWidth();
                    }
                }
                else {
                    currentX += elemSpacing;
                }
                m_profileSpace->SetPosition(wxPoint(currentX, elementY));
                m_profileSpace->SetSize(profileRequestedWidth, innerHeight);
                m_profileSpace->Show(true);
            }
            else {
                m_profileSpace->Show(false);
            }
        }
        else if (funcProfileSpacerAutoExpand && funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            if (tabFuncSpacerVisible) {
                m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
                m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), innerHeight);
                currentX += m_tabFunctionSpacer->GetSpacerWidth();
            }
            else {
                currentX += elemSpacing;
            }

            m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, innerHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
            int autoSpacerWidth = rightBoundary - currentX - profileRequestedWidth;
            autoSpacerWidth = wxMax(m_functionProfileSpacer->GetSpacerWidth(), autoSpacerWidth);
            m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            m_functionProfileSpacer->SetSize(autoSpacerWidth, innerHeight);
            currentX += autoSpacerWidth;
            m_profileSpace->SetPosition(wxPoint(currentX, elementY));
            m_profileSpace->SetSize(profileRequestedWidth, innerHeight);
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
                }
                else {
                    reservedWidth += elemSpacing;
                }
            }

            if (profileSpaceIsEffectivelyVisible) {
                reservedWidth += profileRequestedWidth;
                if (funcSpaceIsEffectivelyVisible) {
                    if (funcProfileSpacerVisible) {
                        reservedWidth += m_functionProfileSpacer->GetSpacerWidth();
                    }
                    else {
                        reservedWidth += elemSpacing;
                    }
                }
                else {
                    if (tabFuncSpacerVisible) {
                        reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
                    }
                    else {
                        reservedWidth += elemSpacing;
                    }
                }
            }

            availableWidthForTabs = wxMax(0, availableWidthForTabs - reservedWidth);

            int tabsWidth = 0;
            if (m_pages.size() == 1 && tabsNeededWidth > 0) { // If only one page AND it needs some width
                tabsWidth = availableWidthForTabs; // It takes all available tab space
            }
            else { // Multiple pages, or no pages needing width
                tabsWidth = wxMin(tabsNeededWidth, availableWidthForTabs);
            }
            tabsWidth = wxMax(0, tabsWidth); // Ensure not negative

            if (tabsWidth > 0) {
                m_tabAreaRect = wxRect(currentX, elementY, tabsWidth, barStripHeight);
                currentX += tabsWidth;
            }
            else {
                m_tabAreaRect = wxRect();
            }

            if (tabFuncSpacerVisible && funcSpaceIsEffectivelyVisible) {
                m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
                m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), innerHeight);
                currentX += m_tabFunctionSpacer->GetSpacerWidth();
            }
            else if (funcSpaceIsEffectivelyVisible) {
                currentX += elemSpacing;
            }

            if (funcSpaceIsEffectivelyVisible) {
                m_functionSpace->SetPosition(wxPoint(currentX, elementY));
                m_functionSpace->SetSize(funcRequestedWidth, innerHeight);
                m_functionSpace->Show(true);
                currentX += funcRequestedWidth;
            }
            else {
                m_functionSpace->Show(false);
            }

            if (funcProfileSpacerVisible && funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
                m_functionProfileSpacer->SetSize(m_functionProfileSpacer->GetSpacerWidth(), innerHeight);
                currentX += m_functionProfileSpacer->GetSpacerWidth();
            }
            else if (funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                currentX += elemSpacing;
            }
            else if (!funcSpaceIsEffectivelyVisible && tabFuncSpacerVisible && profileSpaceIsEffectivelyVisible) {
                m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
                m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), innerHeight);
                currentX += m_tabFunctionSpacer->GetSpacerWidth();
            }
            else if (!funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
                currentX += elemSpacing;
            }

            if (profileSpaceIsEffectivelyVisible) {
                m_profileSpace->SetPosition(wxPoint(currentX, elementY));
                m_profileSpace->SetSize(profileRequestedWidth, innerHeight);
                m_profileSpace->Show(true);
                currentX += profileRequestedWidth;
            }
            else {
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
            }
            else {
                reservedWidth += elemSpacing;
            }
        }

        if (profileSpaceIsEffectivelyVisible) {
            reservedWidth += profileRequestedWidth;
            if (funcSpaceIsEffectivelyVisible) {
                if (funcProfileSpacerVisible) {
                    reservedWidth += m_functionProfileSpacer->GetSpacerWidth();
                }
                else {
                    reservedWidth += elemSpacing;
                }
            }
            else {
                if (tabFuncSpacerVisible) {
                    reservedWidth += m_tabFunctionSpacer->GetSpacerWidth();
                }
                else {
                    reservedWidth += elemSpacing;
                }
            }
        }

        availableWidthForTabs = wxMax(0, availableWidthForTabs - reservedWidth);

        int tabsWidth = 0;
        if (m_pages.size() == 1 && tabsNeededWidth > 0) { // If only one page AND it needs some width
            tabsWidth = availableWidthForTabs; // It takes all available tab space
        }
        else { // Multiple pages, or no pages needing width
            tabsWidth = wxMin(tabsNeededWidth, availableWidthForTabs);
        }
        tabsWidth = wxMax(0, tabsWidth); // Ensure not negative

        if (tabsWidth > 0) {
            m_tabAreaRect = wxRect(currentX, elementY, tabsWidth, barStripHeight);
            currentX += tabsWidth;
        }
        else {
            m_tabAreaRect = wxRect();
        }

        if (tabFuncSpacerVisible && funcSpaceIsEffectivelyVisible) {
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), innerHeight);
            currentX += m_tabFunctionSpacer->GetSpacerWidth();
        }
        else if (funcSpaceIsEffectivelyVisible) {
            currentX += elemSpacing;
        }

        if (funcSpaceIsEffectivelyVisible) {
            m_functionSpace->SetPosition(wxPoint(currentX, elementY));
            m_functionSpace->SetSize(funcRequestedWidth, innerHeight);
            m_functionSpace->Show(true);
            currentX += funcRequestedWidth;
        }
        else {
            m_functionSpace->Show(false);
        }

        if (funcProfileSpacerVisible && funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            m_functionProfileSpacer->SetPosition(wxPoint(currentX, elementY));
            m_functionProfileSpacer->SetSize(m_functionProfileSpacer->GetSpacerWidth(), innerHeight);
            currentX += m_functionProfileSpacer->GetSpacerWidth();
        }
        else if (funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            currentX += elemSpacing;
        }
        else if (!funcSpaceIsEffectivelyVisible && tabFuncSpacerVisible && profileSpaceIsEffectivelyVisible) {
            m_tabFunctionSpacer->SetPosition(wxPoint(currentX, elementY));
            m_tabFunctionSpacer->SetSize(m_tabFunctionSpacer->GetSpacerWidth(), innerHeight);
            currentX += m_tabFunctionSpacer->GetSpacerWidth();
        }
        else if (!funcSpaceIsEffectivelyVisible && profileSpaceIsEffectivelyVisible) {
            currentX += elemSpacing;
        }

        if (profileSpaceIsEffectivelyVisible) {
            m_profileSpace->SetPosition(wxPoint(currentX, elementY));
            m_profileSpace->SetSize(profileRequestedWidth, innerHeight);
            m_profileSpace->Show(true);
            currentX += profileRequestedWidth;
        }
        else {
            m_profileSpace->Show(false);
        }
    }
    if (sysButtonsWidth > 0) {
        int ctrlX = barClientSz.GetWidth() - barPadding - sysButtonsWidth;
        m_systemButtons->SetPosition(wxPoint(ctrlX, elementY));
        m_systemButtons->SetSize(sysButtonsWidth, innerHeight);
        m_systemButtons->Show(true);
    }
    else {
        m_systemButtons->Show(false);
    }
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

    if (m_functionSpaceCenterAlign && m_functionSpace) {
        if (m_functionSpace->IsShown()) {
            int fxW = m_functionSpace->GetSize().GetWidth();
            int fxX = (barClientSz.GetWidth() - fxW) / 2;
            m_functionSpace->SetPosition(wxPoint(fxX, m_barTopMargin));
        }
    }
    if (m_profileSpaceRightAlign && m_profileSpace) {
        if (m_profileSpace->IsShown()) {
            int profW = m_profileSpace->GetSize().GetWidth();
            int barPadding = CFG_INT("BarPadding", FLATUI_BAR_PADDING);
            int elemSpacing = CFG_INT("BarElementSpacing", FLATUI_BAR_ELEMENT_SPACING);
            int sysButtonsWidth = m_systemButtons && m_systemButtons->IsShown()
                ? m_systemButtons->GetRequiredWidth() : 0;
            int rb = barClientSz.GetWidth() - barPadding;
            if (sysButtonsWidth > 0) rb -= (sysButtonsWidth + elemSpacing);
            int profX = rb - profW;
            m_profileSpace->SetPosition(wxPoint(profX, m_barTopMargin));
        }
    }

    Refresh();
    if (m_profileSpace && m_profileSpace->IsShown()) {
        int profH = m_profileSpace->GetSize().GetHeight();
        LOG_DBG("ProfileSpace height: " + std::to_string(profH), "FlatUIBar");
    }
}

int FlatUIBar::CalculateTabsWidth(wxDC& dc) const
{
    int tabPadding = CFG_INT("BarTabPadding", FLATUI_BAR_TAB_PADDING);
    int tabSpacing = CFG_INT("BarTabSpacing", FLATUI_BAR_TAB_SPACING);
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