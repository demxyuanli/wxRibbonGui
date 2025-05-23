#include "flatui/FlatUIGallery.h"
#include "flatui/FlatUIConstants.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIEventManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>

FlatUIGallery::FlatUIGallery(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY)
{
    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(100, 30));
    Bind(wxEVT_PAINT, &FlatUIGallery::OnPaint, this);
    Bind(wxEVT_LEFT_DOWN, &FlatUIGallery::OnMouseDown, this);
    Bind(wxEVT_SIZE, &FlatUIGallery::OnSize, this);
}

FlatUIGallery::~FlatUIGallery()
{
}

void FlatUIGallery::AddItem(const wxBitmap& bitmap, int id)
{
    Freeze();
    ItemInfo info;
    info.bitmap = bitmap;
    info.id = id;
    m_items.push_back(info);

    // Best size is now determined by DoGetBestSize.
    // We just need to inform the layout system that our best size might have changed.
    InvalidateBestSize();

    wxWindow* parent = GetParent();
    if (parent) {
        // It's usually better to let the panel manage its own updates if needed.
        // panel->UpdatePanelSize(); might still be relevant if panel's size depends on gallery's exact new width.
        FlatUIPanel* panel = dynamic_cast<FlatUIPanel*>(parent);
        if (panel) {
            panel->UpdatePanelSize(); // This will re-query gallery's best size.
        }
        // Alternatively, or in addition, a simple Layout() on parent might suffice if parent is a sizer window.
        // parent->Layout(); 
    }

    Refresh(); // Refresh this gallery control
    Thaw();

    // Optional: Log the new best size for debugging
    wxSize bestSize = GetBestSize();
    LOG_INF("Added item to FlatUIGallery, new best size: " +
        std::to_string(bestSize.GetWidth()) + "x" +
        std::to_string(bestSize.GetHeight()), "FlatUIGallery");
}

wxSize FlatUIGallery::DoGetBestSize() const
{
    int currentCalculatedWidth = FLATUI_GALLERY_HORIZONTAL_MARGIN; // Start with left margin
    int itemCount = 0;
    bool hasItems = false;

    for (const auto& item : m_items) {
        if (item.bitmap.IsOk()) {
            hasItems = true;
            currentCalculatedWidth += item.bitmap.GetWidth();
            if (itemCount > 0) currentCalculatedWidth += FLATUI_GALLERY_ITEM_SPACING; // Use constant
            itemCount++;
        }
    }
    if (hasItems) {
        currentCalculatedWidth += FLATUI_GALLERY_HORIZONTAL_MARGIN; // Add right margin if there were items
    } else {
        currentCalculatedWidth = GetMinSize().GetWidth(); 
        if (currentCalculatedWidth <= 0) currentCalculatedWidth = 100; 
    }
    
    return wxSize(currentCalculatedWidth, FLATUI_GALLERY_TARGET_HEIGHT);
}

void FlatUIGallery::OnSize(wxSizeEvent& evt)
{
    Refresh();
    evt.Skip();
}

void FlatUIGallery::OnPaint(wxPaintEvent& evt)
{
    wxAutoBufferedPaintDC dc(this);
    wxSize size = GetSize();

    dc.SetBackground(FLATUI_PRIMARY_CONTENT_BG_COLOUR);
    dc.Clear();

    // Top, Left, Bottom borders in new bg color (to blend or be effectively invisible if panel borders exist)
    dc.SetPen(wxPen(FLATUI_PRIMARY_CONTENT_BG_COLOUR, 1));
    dc.DrawLine(0, 0, size.GetWidth(), 0); // Top
    dc.DrawLine(0, 0, 0, size.GetHeight()); // Left
    dc.DrawLine(0, size.GetHeight() - 1, size.GetWidth(), size.GetHeight() - 1); // Bottom
    
    // Distinct Right border
    dc.SetPen(wxPen(FLATUI_DEFAULT_BORDER_COLOUR, 1));
    dc.DrawLine(size.GetWidth() - 1, 0, size.GetWidth() - 1, size.GetHeight()); // Right

    int x = FLATUI_GALLERY_HORIZONTAL_MARGIN; // Use constant for left margin
    int y = (size.GetHeight() - FLATUI_BUTTONBAR_ICON_SIZE) / 2; // Center items vertically (assuming icon size for items)
    // If items can have variable heights, this vertical centering needs to be per item or based on max item height.
    // For now, assuming all items are icon-like and their height is roughly FLATUI_BUTTONBAR_ICON_SIZE for centering purposes.

    for (auto& item : m_items) {
        if (item.bitmap.IsOk()) {
            // Ensure y is calculated to center this specific bitmap if heights vary
            int item_y = (size.GetHeight() - item.bitmap.GetHeight()) / 2;
            dc.DrawBitmap(item.bitmap, x, item_y, true);
            item.rect = wxRect(x, item_y, item.bitmap.GetWidth(), item.bitmap.GetHeight());
            x += item.bitmap.GetWidth() + FLATUI_GALLERY_ITEM_SPACING; // Use constant for spacing
        }
    }

    LOG_DBG("FlatUIGallery OnPaint - Size: " +
        std::to_string(size.GetWidth()) + "x" +
        std::to_string(size.GetHeight()), "FlatUIGallery");
}

void FlatUIGallery::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    LOG_INF("Mouse down event in FlatUIGallery at position: (" +
        std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")", "FlatUIGallery");

    for (const auto& item : m_items) {
        if (item.rect.Contains(pos)) {
            LOG_INF("Clicked on item with ID: " + std::to_string(item.id), "FlatUIGallery");
            wxCommandEvent event(wxEVT_BUTTON, item.id);
            event.SetEventObject(this);
            ProcessWindowEvent(event);
            break;
        }
    }
    evt.Skip();
}