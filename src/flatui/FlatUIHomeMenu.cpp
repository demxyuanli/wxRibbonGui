#include "flatui/FlatUIHomeMenu.h"
#include "flatui/FlatUIFrame.h" // For m_parentFrame
#include "flatui/FlatUIConstants.h"
#include "flatui/FlatUIHomeSpace.h" // Needed for dynamic_cast and OnHomeMenuClosed call
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/settings.h>
#include <wx/dcbuffer.h> // For wxAutoBufferedPaintDC

#include "config/ConstantsConfig.h"
#define CFG_COLOUR(key, def) ConstantsConfig::getInstance().getColourValue(key, def)
#define CFG_INT(key, def)    ConstantsConfig::getInstance().getIntValue(key, def)

wxBEGIN_EVENT_TABLE(FlatUIHomeMenu, wxPopupTransientWindow)
    EVT_MOTION(FlatUIHomeMenu::OnMouseMotion)
    EVT_KILL_FOCUS(FlatUIHomeMenu::OnKillFocus)
wxEND_EVENT_TABLE()

FlatUIHomeMenu::FlatUIHomeMenu(wxWindow* parent, FlatUIFrame* eventSinkFrame)
    : wxPopupTransientWindow(parent, wxBORDER_NONE),
      m_eventSinkFrame(eventSinkFrame)
{
    // Enable paint background style for auto-buffered drawing
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetBackgroundColour(CFG_COLOUR("PrimaryContentBgColour", FLATUI_PRIMARY_CONTENT_BG_COLOUR));

    m_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    m_panel->SetBackgroundStyle(wxBG_STYLE_PAINT);
    m_panel->SetBackgroundColour(CFG_COLOUR("PrimaryContentBgColour", FLATUI_PRIMARY_CONTENT_BG_COLOUR));

    m_itemSizer = new wxBoxSizer(wxVERTICAL);
    m_panel->SetSizer(m_itemSizer);

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(m_panel, 1, wxEXPAND);
    SetSizer(mainSizer);

    m_panel->Bind(wxEVT_PAINT, &FlatUIHomeMenu::OnPaint, this);
}

FlatUIHomeMenu::~FlatUIHomeMenu() 
{
}

void FlatUIHomeMenu::AddMenuItem(const wxString& text, int id, const wxBitmap& icon)
{
    m_menuItems.push_back(FlatHomeMenuItemInfo(text, id, icon));
    // We could build layout incrementally, or all at once before showing.
    // For now, let's assume BuildMenuLayout() is called before Show().
}

void FlatUIHomeMenu::AddSeparator()
{
    m_menuItems.push_back(FlatHomeMenuItemInfo(wxEmptyString, wxID_SEPARATOR, wxNullBitmap, true));
}

void FlatUIHomeMenu::BuildMenuLayout()
{
    m_itemSizer->Clear(true); // Clear previous items if any

    bool hasDynamicItems = !m_menuItems.empty();

    for (const auto& itemInfo : m_menuItems) {
        if (itemInfo.isSeparator) {
            wxPanel* separator = new wxPanel(m_panel, wxID_ANY, wxDefaultPosition, wxSize(CFG_INT("HomeMenuWidth", HOMEMENU_WIDTH) - 10, 1));
            separator->SetBackgroundColour(CFG_COLOUR("BarTabBorderColour", FLATUI_BAR_TAB_BORDER_COLOUR));
            m_itemSizer->Add(separator, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, (CFG_INT("HomeMenuSeparatorHeight", HOMEMENU_SEPARATOR_HEIGHT)-1)/2 );
        } else {
            wxPanel* itemPanel = new wxPanel(m_panel, wxID_ANY);
            itemPanel->SetBackgroundColour(FLATUI_PRIMARY_CONTENT_BG_COLOUR);
            wxBoxSizer* hsizer = new wxBoxSizer(wxHORIZONTAL);
            
            if(itemInfo.icon.IsOk()){
                wxStaticBitmap* sb = new wxStaticBitmap(itemPanel, wxID_ANY, itemInfo.icon);
                hsizer->Add(sb, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
            }
            wxStaticText* st = new wxStaticText(itemPanel, itemInfo.id, itemInfo.text);
            st->SetFont(GetFlatUIDefaultFont());
            st->SetForegroundColour(*wxBLACK);
            hsizer->Add(st, 1, wxLEFT | wxEXPAND, 5);
            itemPanel->SetSizer(hsizer);

            m_itemSizer->Add(itemPanel, 0, wxEXPAND | wxALL, 2);
            itemPanel->SetMinSize(wxSize(CFG_INT("HomeMenuWidth", HOMEMENU_WIDTH), CFG_INT("HomeMenuHeight", HOMEMENU_HEIGHT)));

            itemPanel->Bind(wxEVT_LEFT_DOWN, [this, item_id = itemInfo.id](wxMouseEvent& event) {
                if (item_id == wxID_EXIT) {
                    SendItemCommand(item_id);
                    event.Skip(); // Still skip to allow any other low-level handlers
                }
                SendItemCommand(item_id);
                Close();
                event.Skip(); 
            });
             st->Bind(wxEVT_LEFT_DOWN, [this, item_id = itemInfo.id](wxMouseEvent& event) {
                if (item_id == wxID_EXIT) {
                    event.SetId(item_id); // SetId might still be useful if other handlers check it
                    event.Skip();
                    return; // Do nothing further for EXIT
                }
                SendItemCommand(item_id);
                Close();
                event.SetId(item_id); 
                event.Skip(); 
            });
        }
    }

    if (hasDynamicItems) {
        // Add a separator if there were dynamic items and we're about to add fixed ones
        wxPanel* separator = new wxPanel(m_panel, wxID_ANY, wxDefaultPosition, wxSize(CFG_INT("HomeMenuWidth", HOMEMENU_WIDTH) - 10, 1));
        separator->SetBackgroundColour(CFG_COLOUR("BarTabBorderColour", FLATUI_BAR_TAB_BORDER_COLOUR));
        // Add a bit more vertical margin for this separator
        m_itemSizer->Add(separator, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, CFG_INT("HomeMenuSeparatorHeight", HOMEMENU_SEPARATOR_HEIGHT));
    }

    m_itemSizer->AddStretchSpacer(1); // This will push subsequent items to the bottom

    // Helper lambda to create and add menu item panels (similar to the loop above)
    auto createAndAddFixedMenuItemPanel = 
        [this](const wxString& text, int id, const wxBitmap& icon) {
        wxPanel* itemPanel = new wxPanel(m_panel, wxID_ANY);
        itemPanel->SetBackgroundColour(CFG_COLOUR("PrimaryContentBgColour", FLATUI_PRIMARY_CONTENT_BG_COLOUR));
        wxBoxSizer* hsizer = new wxBoxSizer(wxHORIZONTAL);
        
        if(icon.IsOk()){
            // For fixed items, ensure icon is scaled if necessary, though 16x16 is standard
            wxStaticBitmap* sb = new wxStaticBitmap(itemPanel, wxID_ANY, icon);
            hsizer->Add(sb, 0, wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT, 5);
        }
        wxStaticText* st = new wxStaticText(itemPanel, id, text);
        st->SetFont(GetFlatUIDefaultFont());
        st->SetForegroundColour(*wxBLACK);
        hsizer->Add(st, 1, wxLEFT | wxEXPAND, 5);
        itemPanel->SetSizer(hsizer);

        m_itemSizer->Add(itemPanel, 0, wxEXPAND | wxALL, 2);
        itemPanel->SetMinSize(wxSize(CFG_INT("HomeMenuWidth", HOMEMENU_WIDTH), CFG_INT("HomeMenuHeight", HOMEMENU_HEIGHT)));

        itemPanel->Bind(wxEVT_LEFT_DOWN, [this, item_id = id](wxMouseEvent& event) {
            SendItemCommand(item_id);
            Close();
            event.SetId(item_id);
            event.Skip();
        });
        st->Bind(wxEVT_LEFT_DOWN, [this, item_id = id](wxMouseEvent& event) {
            SendItemCommand(item_id);
            Close();
            event.SetId(item_id);
            event.Skip(); 
        });
    };
    
    auto addFixedSeparatorToSizer = [&]() {
        wxPanel* separator = new wxPanel(m_panel, wxID_ANY, wxDefaultPosition, wxSize(CFG_INT("HomeMenuWidth", HOMEMENU_WIDTH) - 10, 1));
        separator->SetBackgroundColour(CFG_COLOUR("BarTabBorderColour", FLATUI_BAR_TAB_BORDER_COLOUR));
        m_itemSizer->Add(separator, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, (CFG_INT("HomeMenuSeparatorHeight", HOMEMENU_SEPARATOR_HEIGHT)-1)/2 );
    };

    // Add fixed bottom items
    createAndAddFixedMenuItemPanel("Settings", wxID_PREFERENCES, 
        wxArtProvider::GetBitmap(wxART_EXECUTABLE_FILE, wxART_MENU, wxSize(16,16)));
    addFixedSeparatorToSizer();
    createAndAddFixedMenuItemPanel("About", wxID_ABOUT, 
        wxArtProvider::GetBitmap(wxART_INFORMATION, wxART_MENU, wxSize(16,16)));
    addFixedSeparatorToSizer();
    createAndAddFixedMenuItemPanel("E&xit", wxID_EXIT, 
        wxArtProvider::GetBitmap(wxART_QUIT, wxART_MENU, wxSize(16,16)));

    m_panel->Layout(); // Tell the panel's sizer to arrange children
    // No Fit() or SetSize() needed here, as frame's size is fixed and panel expands.
}

void FlatUIHomeMenu::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(m_panel);
    int w, h;
    m_panel->GetSize(&w, &h);
    dc.SetPen(wxPen(FLATUI_BAR_TAB_BORDER_COLOUR, 1));
    dc.DrawLine(w - 1, 0, w - 1, h);
}

void FlatUIHomeMenu::OnMouseMotion(wxMouseEvent& event)
{
    event.Skip();
}

void FlatUIHomeMenu::SendItemCommand(int id)
{
    if (m_eventSinkFrame && id != wxID_ANY && id != wxID_SEPARATOR) {
        wxCommandEvent cmdEvent(wxEVT_MENU, id);
        if (id == wxID_EXIT) {
            cmdEvent.SetEventObject(m_eventSinkFrame); 
        } else {
            cmdEvent.SetEventObject(this);
        }
        wxPostEvent(m_eventSinkFrame, cmdEvent);
    }
}

void FlatUIHomeMenu::ShowAt(const wxPoint& pos, int contentHeight)
{
    SetPosition(pos);
    SetSize(wxSize(CFG_INT("HomeMenuWidth", HOMEMENU_WIDTH), contentHeight));
    wxPopupWindow::Show(); // Using Show as Popup() caused issues
    
    // Try to set focus to the popup window itself.
    // It's important that the window is visible and able to receive focus.
    if (IsShownOnScreen()) { // Check if it's actually on screen
        SetFocus(); // Equivalent to this->SetFocus()
    } else {
        // If not shown on screen immediately, SetFocus might fail.
        // This could indicate a deeper issue or a need to defer SetFocus.
        wxLogDebug(wxT("FlatUIHomeMenu::ShowAt - Window not shown on screen when trying to SetFocus."));
    }
}

bool FlatUIHomeMenu::Close(bool force)
{
    if (!IsShown()) return false;

    Hide();
    wxWindow* parent = GetParent();
    if (parent) {
        FlatUIHomeSpace* ownerHomeSpace = dynamic_cast<FlatUIHomeSpace*>(parent);
        if (ownerHomeSpace) {
            ownerHomeSpace->OnHomeMenuClosed(this);
        }
    }
    return true;
}

void FlatUIHomeMenu::OnDismiss()
{
    if (IsShown()) {
        Close(); 
    }
}

void FlatUIHomeMenu::OnKillFocus(wxFocusEvent& event)
{
    // Get the window that is receiving focus
    wxWindow* newFocus = event.GetWindow();

    // Check if the new focus window is this popup itself or one of its children.
    // If focus moves to a child of the popup, we don't want to close it.
    bool focusStillInside = (newFocus == this);
    if (!focusStillInside && newFocus) {
        wxWindow* parent = newFocus->GetParent();
        while(parent) {
            if (parent == this) {
                focusStillInside = true;
                break;
            }
            parent = parent->GetParent();
        }
    }

    if (!focusStillInside) {
        wxLogDebug(wxT("FlatUIHomeMenu::OnKillFocus - Focus moved outside. Closing menu."));
        Close(); // Use our existing Close method which also notifies HomeSpace
    } else {
        wxLogDebug(wxT("FlatUIHomeMenu::OnKillFocus - Focus moved to self or child."));
    }
    
    event.Skip(); // Important to allow default processing too
} 