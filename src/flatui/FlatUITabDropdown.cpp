#include "flatui/FlatUITabDropdown.h"
#include "flatui/FlatUIBar.h"
#include "flatui/FlatUIPage.h"
#include "flatui/FlatUIBarStateManager.h"
#include "config/ConstantsConfig.h"
#include "config/SvgIconManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>

#define CFG_COLOUR(key) ConstantsConfig::getInstance().getColourValue(key)
#define CFG_INT(key)    ConstantsConfig::getInstance().getIntValue(key)

wxBEGIN_EVENT_TABLE(FlatUITabDropdown, wxControl)
    EVT_PAINT(FlatUITabDropdown::OnPaint)
    EVT_LEFT_DOWN(FlatUITabDropdown::OnMouseDown)
    EVT_ENTER_WINDOW(FlatUITabDropdown::OnMouseEnter)
    EVT_LEAVE_WINDOW(FlatUITabDropdown::OnMouseLeave)
    EVT_MENU_RANGE(FlatUITabDropdown::MENU_ID_START, FlatUITabDropdown::MENU_ID_END, FlatUITabDropdown::OnMenuItemSelected)
wxEND_EVENT_TABLE()

FlatUITabDropdown::FlatUITabDropdown(wxWindow* parent, wxWindowID id,
                                   const wxPoint& pos, const wxSize& size, long style)
    : wxControl(parent, id, pos, size, style | wxBORDER_NONE),
      m_parentBar(nullptr),
      m_dropdownMenu(nullptr),
      m_isMouseOver(false),
      m_isPressed(false)
{
    SetName("FlatUITabDropdown");
    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(CFG_COLOUR("BarBackgroundColour"));
    
    CreateMenu();
    
    LOG_INF("Created FlatUITabDropdown", "TabDropdown");
}

FlatUITabDropdown::~FlatUITabDropdown()
{
    if (m_dropdownMenu) {
        delete m_dropdownMenu;
        m_dropdownMenu = nullptr;
    }
    
    LOG_INF("Destroyed FlatUITabDropdown", "TabDropdown");
}

void FlatUITabDropdown::UpdateHiddenTabs(const std::vector<size_t>& hiddenIndices)
{
    m_hiddenTabIndices = hiddenIndices;
    PopulateMenu();
    
    // Show or hide the dropdown based on whether there are hidden tabs
    ShowDropdown(!hiddenIndices.empty());
    
    LOG_INF("Updated hidden tabs: " + std::to_string(hiddenIndices.size()) + " tabs", "TabDropdown");
}

void FlatUITabDropdown::ClearMenu()
{
    if (m_dropdownMenu) {
        // Clear existing menu items
        while (m_dropdownMenu->GetMenuItemCount() > 0) {
            m_dropdownMenu->Destroy(m_dropdownMenu->FindItemByPosition(0));
        }
    }
    // Don't clear m_hiddenTabIndices here as it's managed by UpdateHiddenTabs
}

void FlatUITabDropdown::ShowDropdown(bool show)
{
    if (IsShown() != show) {
        Show(show);
        if (show) {
            Refresh();
        }
        LOG_DBG("Dropdown " + std::string(show ? "shown" : "hidden"), "TabDropdown");
    }
}

bool FlatUITabDropdown::IsDropdownShown() const
{
    return IsShown();
}

void FlatUITabDropdown::SetDropdownRect(const wxRect& rect)
{
    m_dropdownRect = rect;
    SetPosition(rect.GetPosition());
    SetSize(rect.GetSize());
}

wxSize FlatUITabDropdown::DoGetBestSize() const
{
    return wxSize(20, 20); // Default dropdown button size
}

void FlatUITabDropdown::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    DrawDropdownButton(dc);
    event.Skip();
}

void FlatUITabDropdown::OnMouseDown(wxMouseEvent& event)
{
    if (!m_parentBar || m_hiddenTabIndices.empty()) {
        event.Skip();
        return;
    }
    
    m_isPressed = true;
    ShowMenu();
    m_isPressed = false;
    
    // Don't skip the event to prevent interference with menu handling
}

void FlatUITabDropdown::OnMouseEnter(wxMouseEvent& event)
{
    SetMouseOver(true);
    event.Skip();
}

void FlatUITabDropdown::OnMouseLeave(wxMouseEvent& event)
{
    SetMouseOver(false);
    event.Skip();
}

void FlatUITabDropdown::OnMenuItemSelected(wxCommandEvent& event)
{
    if (!m_parentBar) {
        return;
    }
    
    int menuId = event.GetId();
    int menuIndex = menuId - MENU_ID_START;
    
    // Validate menu index
    if (menuIndex >= 0 && menuIndex < static_cast<int>(m_hiddenTabIndices.size())) {
        size_t actualTabIndex = m_hiddenTabIndices[menuIndex];
        
        // Verify the page exists
        FlatUIPage* selectedPage = m_parentBar->GetPage(actualTabIndex);
        if (!selectedPage) {
            return;
        }
        
        // Use the same logic as FlatBarSpaceContainer::HandleTabClick for consistency
        if (m_parentBar->IsBarPinned()) {
            // Pinned state: use SetActivePage directly
            m_parentBar->SetActivePage(actualTabIndex);
        } else {
            // Unpinned state: manually handle like EventDispatcher does
            FlatUIBarStateManager* stateManager = m_parentBar->GetStateManager();
            if (stateManager) {
                stateManager->SetActiveFloatingPage(actualTabIndex);
                stateManager->SetActivePage(actualTabIndex);
                
                // Show the page in float panel
                m_parentBar->ShowPageInFloatPanel(selectedPage);
            }
        }
        
        // Refresh the parent container to update tab display
        wxWindow* parent = GetParent();
        if (parent) {
            parent->Refresh();
        }
    }
    
    // Don't call event.Skip() here to prevent further event processing
    // that might interfere with our tab activation
}

void FlatUITabDropdown::DrawDropdownButton(wxDC& dc)
{
    wxSize size = GetSize();
    
    // Fill background with margins: top margin 4px, bottom margin 0px
    const int TOP_MARGIN = 4;
    const int BOTTOM_MARGIN = 0;
    
    wxColour bgColour = CFG_COLOUR("BarBackgroundColour");
    if (m_isMouseOver) {
        // Slightly darker when mouse over
        bgColour = wxColour(
            wxMax(0, bgColour.Red() - 20),
            wxMax(0, bgColour.Green() - 20),
            wxMax(0, bgColour.Blue() - 20)
        );
    }
    
    // Draw background with vertical margins
    dc.SetBrush(wxBrush(bgColour));
    dc.SetPen(*wxTRANSPARENT_PEN);
    wxRect bgRect(0, TOP_MARGIN, size.GetWidth(), size.GetHeight() - TOP_MARGIN - BOTTOM_MARGIN);
    dc.DrawRectangle(bgRect);
    
    // Draw only the right border line
    dc.SetPen(wxPen(CFG_COLOUR("BarBorderColour"), 1));
    dc.DrawLine(size.GetWidth() - 1, TOP_MARGIN, 
                size.GetWidth() - 1, size.GetHeight() - BOTTOM_MARGIN);
    
    // Draw dropdown arrow in the adjusted area
    DrawDropdownArrow(dc, bgRect);
}

void FlatUITabDropdown::DrawDropdownArrow(wxDC& dc, const wxRect& rect)
{
    // Use down.svg icon with 12x12 size
    wxSize iconSize(12, 12);
    wxBitmap iconBitmap = SvgIconManager::GetInstance().GetIconBitmap("down", iconSize);
    
    if (iconBitmap.IsOk()) {
        // Center the icon in the rect
        int iconX = rect.x + (rect.width - iconSize.GetWidth()) / 2;
        int iconY = rect.y + (rect.height - iconSize.GetHeight()) / 2;
        
        dc.DrawBitmap(iconBitmap, iconX, iconY, true);
    } else {
        // Fallback to text if SVG icon is not available
        wxColour arrowColour = CFG_COLOUR("BarInactiveTextColour");
        dc.SetTextForeground(arrowColour);
        
        wxString dropdownText = "...";
        wxSize textSize = dc.GetTextExtent(dropdownText);
        
        int textX = rect.x + (rect.width - textSize.GetWidth()) / 2;
        int textY = rect.y + (rect.height - textSize.GetHeight()) / 2;
        
        dc.DrawText(dropdownText, textX, textY);
    }
}

void FlatUITabDropdown::CreateMenu()
{
    if (m_dropdownMenu) {
        delete m_dropdownMenu;
    }
    
    m_dropdownMenu = new wxMenu();
    LOG_DBG("Created dropdown menu", "TabDropdown");
}

void FlatUITabDropdown::ShowMenu()
{
    if (!m_dropdownMenu || m_hiddenTabIndices.empty()) {
        return;
    }
    
    // Ensure menu is populated with current hidden tabs
    PopulateMenu();
    
    // Show the popup menu at the bottom of the dropdown button
    PopupMenu(m_dropdownMenu, 0, GetSize().GetHeight());
}

void FlatUITabDropdown::PopulateMenu()
{
    if (!m_dropdownMenu || !m_parentBar) {
        return;
    }
    
    // Clear existing menu items only (not hidden tab indices)
    if (m_dropdownMenu) {
        while (m_dropdownMenu->GetMenuItemCount() > 0) {
            m_dropdownMenu->Destroy(m_dropdownMenu->FindItemByPosition(0));
        }
    }
    
    // Add menu items for hidden tabs
    size_t totalPageCount = m_parentBar->GetPageCount();
    
    for (size_t i = 0; i < m_hiddenTabIndices.size(); ++i) {
        size_t tabIndex = m_hiddenTabIndices[i];
        
        // Check if tab index is within valid range
        if (tabIndex >= totalPageCount) {
            continue;
        }
        
        FlatUIPage* page = m_parentBar->GetPage(tabIndex);
        
        if (page) {
            int menuId = MENU_ID_START + i;
            m_dropdownMenu->Append(menuId, page->GetLabel());
        }
    }
}

void FlatUITabDropdown::SetMouseOver(bool over)
{
    if (m_isMouseOver != over) {
        m_isMouseOver = over;
        // Only refresh if the control is visible
        if (IsShown()) {
            Refresh();
        }
    }
} 