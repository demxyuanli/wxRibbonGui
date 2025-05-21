#include "flatui/FlatUIGallery.h"
#include "flatui/FlatUIPanel.h"
#include "flatui/FlatUIEventManager.h"
#include <string>
#include "logger/Logger.h"


FlatUIGallery::FlatUIGallery(FlatUIPanel* parent)
    : wxControl(parent, wxID_ANY)
{
}

FlatUIGallery::~FlatUIGallery()
{
}

void FlatUIGallery::AddItem(const wxBitmap& bitmap, int id)
{
    ItemInfo info;
    info.bitmap = bitmap;
    info.id = id;
    m_items.push_back(info);
    Refresh();
}

void FlatUIGallery::OnPaint(wxPaintEvent& evt)
{
    wxPaintDC dc(this);
    wxSize size = GetSize();
    dc.SetBackground(wxColour(230, 230, 230));
    dc.Clear();

    int x = 10;
    int y = 10;
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        ItemInfo& info = m_items[i];
        dc.DrawBitmap(info.bitmap, x, y);
        info.rect = wxRect(x, y, info.bitmap.GetWidth(), info.bitmap.GetHeight());
        x += info.bitmap.GetWidth() + 10;
    }
}

void FlatUIGallery::OnMouseDown(wxMouseEvent& evt)
{
    wxPoint pos = evt.GetPosition();
    Logger::getLogger().Log(Logger::LogLevel::INF, "Mouse down event in FlatUIGallery at position: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ")", "FlatUIGallery");
    for (const auto& item : m_items)
    {
        if (item.rect.Contains(pos))
        {
            Logger::getLogger().Log(Logger::LogLevel::INF, "Clicked on item with ID: " + std::to_string(item.id), "FlatUIGallery");
            wxCommandEvent event(wxEVT_BUTTON, item.id);
            event.SetEventObject(this);
            ProcessWindowEvent(event);
            break;
        }
    }
    evt.Skip();
}