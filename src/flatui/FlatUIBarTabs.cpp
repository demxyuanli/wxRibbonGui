#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIPage.h"
#include <wx/dcbuffer.h>
#include <string>
#include "config/ConstantsConfig.h"
#include "logger/Logger.h"

#define CFG_INT(key) ConstantsConfig::getInstance().getIntValue(key)
#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_FONTNAME() ConstantsConfig::getInstance().getDefaultFontFaceName()
#define CFG_DEFAULTFONT() ConstantsConfig::getInstance().getDefaultFont()

void FlatUIBar::PaintTabsArea(wxDC& dc, int availableWidth, int& currentXOffset)
{
    PaintTabs(dc, availableWidth, currentXOffset);
}

void FlatUIBar::HandleTabAreaClick(const wxPoint& pos)
{
    int barH = GetBarHeight();
    int tabStartX = (CFG_INT("BarPadding"));
    if (m_homeSpace && m_homeSpace->IsShown()) {
        tabStartX = m_homeSpace->GetRect().GetRight() + CFG_INT("BarElementSpacing");
    }
    int tabEndX = GetClientSize().GetWidth() - (CFG_INT("BarPadding"));
    if (m_systemButtons && m_systemButtons->IsShown()) {
        tabEndX = m_systemButtons->GetRect().GetLeft() - CFG_INT("BarElementSpacing");
    }
    wxClientDC dc(this);
    if (pos.y >= 0 && pos.y < barH && pos.x >= tabStartX && pos.x < tabEndX) {
        int currentX = tabStartX;
        bool clickedOnTab = false;
        
        for (size_t i = 0; i < m_pages.size(); ++i) {
            if (!m_pages[i]) continue;
            FlatUIPage* page = m_pages[i];
            wxSize labelSize = dc.GetTextExtent(page->GetLabel());
            int tabWidth = labelSize.GetWidth() + CFG_INT("BarTabPadding") * 2;
            wxRect rect(currentX, 0, tabWidth, barH);
            if (rect.Contains(pos)) {
                clickedOnTab = true;
                
                // If pinned, check if tab is already selected and handle accordingly
                if (m_isGlobalPinned) {
                    // Check if this tab is already selected to avoid unnecessary actions
                    if (i == m_activePage) {
                        LOG_INF("Tab " + std::to_string(i) + " is already active in pinned mode, ignoring click", "FlatUIBar");
                        break;
                    }
                    // Set the active page directly
                    SetActivePage(i);
                }
                else {
                    // Unpinned: Always handle tab clicks to show/update float panel
                    FlatUIPage* clickedPage = m_pages[i];
                    
                    // Update both floating page and active page for consistency
                    m_activeFloatingPage = i;
                    m_activePage = i; // Keep m_activePage synchronized for pin state transitions
                    
                    if (m_floatPanel && m_floatPanel->IsShown()) {
                        // Float panel is already shown, just update the content
                        LOG_INF("Updating float panel content to tab " + std::to_string(i), "FlatUIBar");
                        m_floatPanel->SetPageContent(clickedPage);
                    } else {
                        // Float panel is not shown, show it with the clicked page
                        LOG_INF("Showing float panel with tab " + std::to_string(i), "FlatUIBar");
                        ShowPageInFloatPanel(clickedPage);
                    }
                    
                    // Refresh to update tab visual state
                    Refresh();
                }
                break;
            }
            currentX += tabWidth + CFG_INT("BarTabSpacing");
            if (currentX >= tabEndX) break;
        }
        
        // If clicked in tab area but not on any specific tab, hide float panel (if unpinned)
        if (!clickedOnTab && !m_isGlobalPinned) {
            HideFloatPanel();
        }
    }
}

void FlatUIBar::PaintTabs(wxDC& dc, int availableTotalWidth, int& currentXOffsetInOut)
{
    int tabYPos = m_barTopMargin + m_tabTopSpacing;  // Use top margin
    int tabPadding = CFG_INT("BarTabPadding");
    int tabSpacing = CFG_INT("BarTabSpacing");
    int barEffectiveHeight = GetBarHeight() - m_tabTopSpacing; 
    int initialXOffset = currentXOffsetInOut;

    dc.SetFont(CFG_DEFAULTFONT());

    for (size_t i = 0; i < m_pages.size(); ++i)
    {
        if (!m_pages[i]) continue;

        FlatUIPage* page = m_pages[i];
        wxString label = page->GetLabel();
        wxSize labelSize = dc.GetTextExtent(label);
        int tabWidth = labelSize.GetWidth() + tabPadding * 2;

        if ((currentXOffsetInOut + tabWidth) > (initialXOffset + availableTotalWidth) && i > 0) {
            break;
        }

        wxRect tabRect(currentXOffsetInOut, tabYPos, tabWidth, barEffectiveHeight);

        // Determine if this tab should be drawn as active
        bool isActiveTab = false;
        if (m_isGlobalPinned) {
            isActiveTab = (i == m_activePage);
        } else {
            isActiveTab = (i == m_activeFloatingPage);
        }

        if (isActiveTab)
        {
            dc.SetBrush(wxBrush(m_activeTabBgColour));
            dc.SetTextForeground(m_activeTabTextColour);

            // Draw tab based on style
            switch (m_tabStyle) {
            case TabStyle::DEFAULT:
                // Fill background of active tab (excluding the top border)
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.DrawRectangle(tabRect.x, tabRect.y + m_tabBorderTop, tabRect.width, tabRect.height - m_tabBorderTop);

                // Draw borders based on border style
                if (m_tabBorderStyle == TabBorderStyle::SOLID) {
                    // Use simple line drawing for solid borders
                    if (m_tabBorderTop > 0) {
                        dc.SetPen(wxPen(m_tabBorderTopColour, m_tabBorderTop));
                        dc.DrawLine(tabRect.GetLeft(), tabRect.GetTop() + m_tabBorderTop / 2,
                            tabRect.GetRight() + 1, tabRect.GetTop() + m_tabBorderTop / 2);

                    }
                    if (m_tabBorderLeft > 0) {
                        dc.SetPen(wxPen(m_tabBorderLeftColour, m_tabBorderLeft));
                        dc.DrawLine(tabRect.GetLeft() , tabRect.GetTop() + m_tabBorderTop,
                            tabRect.GetLeft() , tabRect.GetBottom());
                    }
                    if (m_tabBorderRight > 0) {
                        dc.SetPen(wxPen(m_tabBorderRightColour, m_tabBorderRight));
                        dc.DrawLine(tabRect.GetRight() + 1 , tabRect.GetTop() + m_tabBorderTop,
                            tabRect.GetRight() + 1, tabRect.GetBottom());
                    }
                }
                else {
                    // Use DrawTabBorder for other border styles
                    DrawTabBorder(dc, tabRect, true);
                }
                break;

            case TabStyle::UNDERLINE:
                // No background fill for underline style
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.SetPen(*wxTRANSPARENT_PEN);

                // Draw bottom border only
                if (m_tabBorderBottom > 0) {
                    dc.SetPen(wxPen(m_tabBorderBottomColour, m_tabBorderBottom));
                    dc.DrawLine(tabRect.GetLeft(), tabRect.GetBottom() - m_tabBorderBottom / 2,
                        tabRect.GetRight() + 1, tabRect.GetBottom() - m_tabBorderBottom / 2);
                }
                break;

            case TabStyle::BUTTON:
                // Fill background
                dc.SetPen(*wxTRANSPARENT_PEN);
                dc.DrawRectangle(tabRect);

                // Draw all borders
                dc.SetPen(wxPen(m_tabBorderColour, 1));
                if (m_tabBorderTop > 0) {
                    dc.SetPen(wxPen(m_tabBorderColour, m_tabBorderTop));
                    dc.DrawLine(tabRect.GetLeft(), tabRect.GetTop(),
                        tabRect.GetRight() + 1, tabRect.GetTop());
                }
                if (m_tabBorderBottom > 0) {
                    dc.SetPen(wxPen(m_tabBorderColour, m_tabBorderBottom));
                    dc.DrawLine(tabRect.GetLeft(), tabRect.GetBottom(),
                        tabRect.GetRight() + 1, tabRect.GetBottom());
                }
                if (m_tabBorderLeft > 0) {
                    dc.SetPen(wxPen(m_tabBorderColour, m_tabBorderLeft));
                    dc.DrawLine(tabRect.GetLeft(), tabRect.GetTop(),
                        tabRect.GetLeft(), tabRect.GetBottom() + 1);
                }
                if (m_tabBorderRight > 0) {
                    dc.SetPen(wxPen(m_tabBorderColour, m_tabBorderRight));
                    dc.DrawLine(tabRect.GetRight(), tabRect.GetTop(),
                        tabRect.GetRight(), tabRect.GetBottom() + 1);
                }
                break;

            case TabStyle::FLAT:
                // No background, no borders, only text color changes
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.SetPen(*wxTRANSPARENT_PEN);
                break;
            }
        }
        else // Inactive Tab
        {
            // No background fill (transparent)
            // No border
            dc.SetBrush(*wxTRANSPARENT_BRUSH);
            dc.SetPen(*wxTRANSPARENT_PEN);
            dc.SetTextForeground(m_inactiveTabTextColour);
        }

        dc.DrawText(label, currentXOffsetInOut + tabPadding, tabYPos + (barEffectiveHeight - labelSize.GetHeight()) / 2);
        currentXOffsetInOut += tabWidth + tabSpacing;
    }
}

void FlatUIBar::OnMouseDown(wxMouseEvent& evt)
{
    HandleTabAreaClick(evt.GetPosition());
    evt.Skip();
}

void FlatUIBar::SetTabStyle(TabStyle style)
{
    m_tabStyle = style;

    // Set default border widths based on style
    switch (style) {
    case TabStyle::DEFAULT:
        m_tabBorderTop = 2;
        m_tabBorderBottom = 0;
        m_tabBorderLeft = 1;
        m_tabBorderRight = 1;
        break;
    case TabStyle::UNDERLINE:
        m_tabBorderTop = 0;
        m_tabBorderBottom = 2;
        m_tabBorderLeft = 0;
        m_tabBorderRight = 0;
        break;
    case TabStyle::BUTTON:
        m_tabBorderTop = 1;
        m_tabBorderBottom = 1;
        m_tabBorderLeft = 1;
        m_tabBorderRight = 1;
        break;
    case TabStyle::FLAT:
        m_tabBorderTop = 1;
        m_tabBorderBottom = 0;
        m_tabBorderLeft = 0;
        m_tabBorderRight = 0;
        break;
    }

    Refresh();
}

void FlatUIBar::SetTabBorderWidths(int top, int bottom, int left, int right)
{
    m_tabBorderTop = top;
    m_tabBorderBottom = bottom;
    m_tabBorderLeft = left;
    m_tabBorderRight = right;
    Refresh();
}


void FlatUIBar::SetTabBorderColour(const wxColour& colour)
{
    m_tabBorderColour = colour;
    // Also update individual border colors for backward compatibility
    m_tabBorderTopColour = colour;
    m_tabBorderBottomColour = colour;
    m_tabBorderLeftColour = colour;
    m_tabBorderRightColour = colour;
    Refresh();
}

void FlatUIBar::SetActiveTabBackgroundColour(const wxColour& colour)
{
    m_activeTabBgColour = colour;
    Refresh();
}

void FlatUIBar::SetActiveTabTextColour(const wxColour& colour)
{
    m_activeTabTextColour = colour;
    Refresh();
}

void FlatUIBar::SetInactiveTabTextColour(const wxColour& colour)
{
    m_inactiveTabTextColour = colour;
    Refresh();
}

void FlatUIBar::SetTabBorderStyle(TabBorderStyle style)
{
    m_tabBorderStyle = style;
    Refresh();
}

void FlatUIBar::SetTabCornerRadius(int radius)
{
    m_tabCornerRadius = radius;
    Refresh();
}

void FlatUIBar::SetTabBorderTopColour(const wxColour& colour)
{
    m_tabBorderTopColour = colour;
    Refresh();
}

void FlatUIBar::SetTabBorderBottomColour(const wxColour& colour)
{
    m_tabBorderBottomColour = colour;
    Refresh();
}

void FlatUIBar::SetTabBorderLeftColour(const wxColour& colour)
{
    m_tabBorderLeftColour = colour;
    Refresh();
}

void FlatUIBar::SetTabBorderRightColour(const wxColour& colour)
{
    m_tabBorderRightColour = colour;
    Refresh();
}

void FlatUIBar::SetTabBorderTopWidth(int width)
{
    m_tabBorderTop = width;
    Refresh();
}

void FlatUIBar::SetTabBorderBottomWidth(int width)
{
    m_tabBorderBottom = width;
    Refresh();
}

void FlatUIBar::SetTabBorderLeftWidth(int width)
{
    m_tabBorderLeft = width;
    Refresh();
}

void FlatUIBar::SetTabBorderRightWidth(int width)
{
    m_tabBorderRight = width;
    Refresh();
}

void FlatUIBar::SetBarTopMargin(int margin)
{
    m_barTopMargin = margin;
    if (IsShown()) {
        UpdateElementPositionsAndSizes(GetClientSize());
        Refresh();
    }
}

void FlatUIBar::SetBarBottomMargin(int margin)
{
	m_barBottomMargin = margin;
	if (IsShown()) {
		UpdateElementPositionsAndSizes(GetClientSize());
		Refresh();
	}
}