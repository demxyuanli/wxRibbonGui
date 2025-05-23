#include "flatui/FlatUIHomeMenu.h"
#include "flatui/FlatFrame.h" // For m_parentFrame
#include "flatui/FlatUIConstants.h"
#include "flatui/FlatUIHomeSpace.h" // Needed for dynamic_cast and OnHomeMenuClosed call
#include <wx/stattext.h>
#include <wx/bmpbuttn.h>
#include <wx/sizer.h>

wxBEGIN_EVENT_TABLE(FlatUIHomeMenu, wxPopupWindow)
    EVT_MOTION(FlatUIHomeMenu::OnMouseMotion)
wxEND_EVENT_TABLE()

FlatUIHomeMenu::FlatUIHomeMenu(wxWindow* parent, FlatFrame* eventSinkFrame)
    : wxPopupWindow(parent, wxBORDER_NONE),
      m_eventSinkFrame(eventSinkFrame)
{
    SetBackgroundColour(FLATUI_PRIMARY_CONTENT_BG_COLOUR);

    m_panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    m_panel->SetBackgroundColour(FLATUI_PRIMARY_CONTENT_BG_COLOUR);
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
            wxPanel* separator = new wxPanel(m_panel, wxID_ANY, wxDefaultPosition, wxSize(MENU_WIDTH -10, 1));
            separator->SetBackgroundColour(FLATUI_BAR_TAB_BORDER_COLOUR);
            m_itemSizer->Add(separator, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, (SEPARATOR_HEIGHT-1)/2 );
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
            itemPanel->SetMinSize(wxSize(MENU_WIDTH, ITEM_HEIGHT));

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
        wxPanel* separator = new wxPanel(m_panel, wxID_ANY, wxDefaultPosition, wxSize(MENU_WIDTH - 10, 1));
        separator->SetBackgroundColour(FLATUI_BAR_TAB_BORDER_COLOUR);
        // Add a bit more vertical margin for this separator
        m_itemSizer->Add(separator, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, SEPARATOR_HEIGHT); 
    }

    m_itemSizer->AddStretchSpacer(1); // This will push subsequent items to the bottom

    // Helper lambda to create and add menu item panels (similar to the loop above)
    auto createAndAddFixedMenuItemPanel = 
        [this](const wxString& text, int id, const wxBitmap& icon) {
        wxPanel* itemPanel = new wxPanel(m_panel, wxID_ANY);
        itemPanel->SetBackgroundColour(FLATUI_PRIMARY_CONTENT_BG_COLOUR);
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
        itemPanel->SetMinSize(wxSize(MENU_WIDTH, ITEM_HEIGHT));

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
        wxPanel* separator = new wxPanel(m_panel, wxID_ANY, wxDefaultPosition, wxSize(MENU_WIDTH -10, 1));
        separator->SetBackgroundColour(FLATUI_BAR_TAB_BORDER_COLOUR);
        m_itemSizer->Add(separator, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxBOTTOM, (SEPARATOR_HEIGHT-1)/2 );
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
    // This paint event is for m_panel now.
    // Custom drawing of items would happen here if not using wxStaticText etc.
    // For now, wxStaticText and wxStaticBitmap handle their own drawing.
    wxPaintDC dc(m_panel);
    // If we wanted to draw items manually:
    // int y_offset = 0;
    // for (auto& itemInfo : m_menuItems) {
    //    if (itemInfo.isSeparator) { /* draw separator */ y_offset += SEPARATOR_HEIGHT; }
    //    else { /* draw item text/icon */ y_offset += ITEM_HEIGHT; }
    //    itemInfo.rect = wxRect(0, y_offset - ITEM_HEIGHT, MENU_WIDTH, ITEM_HEIGHT); // Update rect for hit test
    // }
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
    SetSize(wxSize(MENU_WIDTH, contentHeight));
    wxPopupWindow::Show();
}

bool FlatUIHomeMenu::Close(bool force)
{
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
    Hide();
    wxWindow* parent = GetParent();
    if (parent) {
        FlatUIHomeSpace* ownerHomeSpace = dynamic_cast<FlatUIHomeSpace*>(parent);
        if (ownerHomeSpace) {
            ownerHomeSpace->OnHomeMenuClosed(this);
        }
    }
} 