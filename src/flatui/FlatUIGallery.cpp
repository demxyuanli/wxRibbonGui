#include "flatui/FlatUIGallery.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIEventManager.h"
#include "logger/Logger.h"
#include <wx/dcbuffer.h>

FlatUIGallery::FlatUIGallery(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY)
{
    SetDoubleBuffered(true);
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetMinSize(wxSize(100, 60));
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

    if (bitmap.IsOk()) {
        int totalWidth = 10;
        int maxHeight = 0;
        for (const auto& item : m_items) {
            if (item.bitmap.IsOk()) {
                totalWidth += item.bitmap.GetWidth() + 10;
                maxHeight = wxMax(maxHeight, item.bitmap.GetHeight());
            }
        }
        int totalHeight = maxHeight + 20;
        SetMinSize(wxSize(totalWidth, totalHeight));

        wxWindow* parent = GetParent();
        if (parent) {
            FlatUIPanel* panel = dynamic_cast<FlatUIPanel*>(parent);
            if (panel) {
                panel->UpdatePanelSize();
            }
        }
    }

    Refresh();
    Thaw();

    LOG_INF("Added item to FlatUIGallery, new size: " +
        std::to_string(GetMinSize().GetWidth()) + "x" +
        std::to_string(GetMinSize().GetHeight()), "FlatUIGallery");
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

    dc.SetBackground(wxColour(230, 230, 230));
    dc.Clear();

    dc.SetPen(wxPen(wxColour(180, 180, 180), 1));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());

    int x = 10;
    int y = 10;
    for (auto& item : m_items) {
        if (item.bitmap.IsOk()) {
            dc.DrawBitmap(item.bitmap, x, y, true);
            item.rect = wxRect(x, y, item.bitmap.GetWidth(), item.bitmap.GetHeight());
            x += item.bitmap.GetWidth() + 10;
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